#include "antpch.h"
#include "ScriptEngine.h"
#include "ScriptUtils.h"
#include "Ant/Utilities/StringUtils.h"
#include "Ant/Scene/Entity.h"
#include "Ant/Core/Hash.h"
#include "ScriptGlue.h"
#include "ScriptCache.h"
#include "ScriptProfiler.h"
#include "ScriptBuilder.h"
#include "Ant/Editor/EditorConsolePanel.h"
#include "Ant/Asset/AssetManager.h"
#include "Ant/Scene/Prefab.h"
#include "Ant/Renderer/SceneRenderer.h"
#include "Ant/Editor/ApplicationSettings.h"

#include "Ant/Debug/Profiler.h"

#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include <stack>

namespace Ant {

	struct ScriptEngineState
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* ScriptsDomain = nullptr;
		MonoDomain* NewScriptsDomain = nullptr;

		Ref<AssemblyInfo> CoreAssemblyInfo = nullptr;
		Ref<AssemblyInfo> AppAssemblyInfo = nullptr;
		MonoAssembly* OldCoreAssembly = nullptr;

		std::unordered_map<AssemblyMetadata, MonoAssembly*> ReferencedAssemblies;

		bool IsMonoInitialized = false;
		bool PostLoadCleanup = false;
		ScriptEngineConfig Config;

		Ref<Scene> SceneContext = nullptr;
		Ref<SceneRenderer> ActiveSceneRenderer = nullptr;
		ScriptEntityMap ScriptEntities;
		ScriptInstanceMap ScriptInstances;
		std::stack<Entity> RuntimeDuplicatedScriptEntities;
		bool ReloadAppAssembly = false;

		std::unordered_map<UUID, std::unordered_map<uint32_t, Ref<FieldStorageBase>>> FieldMap;
	};

	static ScriptEngineState* s_State = nullptr;
	void ScriptEngine::Init(const ScriptEngineConfig& config)
	{
		ANT_CORE_ASSERT(!s_State, "[ScriptEngine]: Trying to call ScriptEngine::Init multiple times!");
		s_State = anew ScriptEngineState();
		s_State->Config = config;
		s_State->CoreAssemblyInfo = Ref<AssemblyInfo>::Create();
		s_State->AppAssemblyInfo = Ref<AssemblyInfo>::Create();

		FileSystem::AddFileSystemChangedCallback(ScriptEngine::OnFileSystemChanged);

		ScriptProfiler::Init();
		InitMono();
		GCManager::Init();

		if (!LoadCoreAssembly())
		{
			// NOTE: In theory this should work for all Visual Studio 2022 instances as long as it's installed in the default location!
			ANT_CORE_INFO_TAG("ScriptEngine", "Failed to load Ants C# core, attempting to build automatically using MSBuild");

			auto scriptCoreAssemblyFile = std::filesystem::current_path().parent_path() / "Ant-ScriptCore" / "Ant-ScriptCore.csproj";
			ScriptBuilder::BuildCSProject(scriptCoreAssemblyFile);

			if (!LoadCoreAssembly())
				ANT_CORE_FATAL_TAG("ScriptEngine", "Failed to build Ant-ScriptCore! Most likely there were compile errors!");
		}
	}

	void ScriptEngine::Shutdown()
	{
		for (auto& [sceneID, entityInstances] : s_State->ScriptEntities)
		{
			auto scene = Scene::GetScene(sceneID);

			if (!scene)
				continue;

			for (auto& entityID : entityInstances)
				ShutdownScriptEntity(scene->TryGetEntityWithUUID(entityID));

			entityInstances.clear();
		}
		s_State->ScriptEntities.clear();
		s_State->FieldMap.clear();

		s_State->SceneContext = nullptr;

		ShutdownMono();
		ScriptProfiler::Shutdown();

		adelete s_State;
		s_State = nullptr;
	}

	void ScriptEngine::InitializeRuntime()
	{
		ANT_PROFILE_FUNC();
		ANT_CORE_ASSERT(s_State->SceneContext, "Tring to initialize script runtime without setting the scene context!");
		ANT_CORE_ASSERT(s_State->SceneContext->IsPlaying(), "Don't call ScriptEngine::InitializeRuntime if the scene isn't being played!");

		auto view = s_State->SceneContext->GetAllEntitiesWith<IDComponent, ScriptComponent>();
		for (auto enttID : view)
		{
			Entity entity = s_State->SceneContext->GetEntityWithUUID(view.get<IDComponent>(enttID).ID);
			RuntimeInitializeScriptEntity(entity);
		}
	}

	void ScriptEngine::ShutdownRuntime()
	{
		ANT_PROFILE_FUNC();

		UUID runtimeSceneID = s_State->SceneContext->GetUUID();
		ANT_CORE_INFO_TAG("ScriptEngine", "Shutting down runtime scene {0}, with {1} scripts active.", runtimeSceneID, s_State->ScriptInstances.size());

		for (auto it = s_State->ScriptInstances.begin(); it != s_State->ScriptInstances.end(); )
		{
			Entity entity = s_State->SceneContext->TryGetEntityWithUUID(it->first);

			if (!entity)
			{
				it++;
				continue;
			}

			ShutdownRuntimeInstance(entity);

			it = s_State->ScriptInstances.erase(it);
		}

		// Why no clear for stack?
		while (s_State->RuntimeDuplicatedScriptEntities.size() > 0)
			s_State->RuntimeDuplicatedScriptEntities.pop();

		GCManager::CollectGarbage();
	}

	void ScriptEngine::ShutdownRuntimeInstance(Entity entity)
	{
		if (!entity.HasComponent<ScriptComponent>())
			return;

		const auto scriptComponent = entity.GetComponent<ScriptComponent>();

		CallMethod(scriptComponent.ManagedInstance, "OnDestroyInternal");

		for (auto fieldID : scriptComponent.FieldIDs)
		{
			Ref<FieldStorageBase> fieldStorage = s_State->FieldMap[entity.GetUUID()][fieldID];
			fieldStorage->SetRuntimeInstance(nullptr);
		}

		GCManager::ReleaseObjectReference(scriptComponent.ManagedInstance);
		entity.GetComponent<ScriptComponent>().ManagedInstance = nullptr;
		entity.GetComponent<ScriptComponent>().IsRuntimeInitialized = false;
	}

	bool ScriptEngine::ReloadAppAssembly(const bool scheduleReload)
	{
		ANT_PROFILE_FUNC();

		if (scheduleReload)
		{
			s_State->ReloadAppAssembly = true;
			return false;
		}

		ANT_CORE_INFO_TAG("ScriptEngine", "Reloading {0}", Project::GetScriptModuleFilePath());

		// Cache old field values and destroy all previous script instances
		std::unordered_map<UUID, std::unordered_map<UUID, std::unordered_map<uint32_t, Buffer>>> oldFieldValues;
		for (auto& [sceneID, entityMap] : s_State->ScriptEntities)
		{
			auto scene = Scene::GetScene(sceneID);

			if (!scene)
				continue;

			for (const auto& entityID : entityMap)
			{
				Entity entity = scene->TryGetEntityWithUUID(entityID);

				if (!entity.HasComponent<ScriptComponent>())
					continue;

				const auto sc = entity.GetComponent<ScriptComponent>();
				ManagedClass* managedClass = ScriptCache::GetManagedClassByID(GetScriptClassIDFromComponent(sc));
				oldFieldValues[sceneID][entityID] = std::unordered_map<uint32_t, Buffer>();

				for (auto fieldID : sc.FieldIDs)
				{
					Ref<FieldStorageBase> storage = s_State->FieldMap[entityID][fieldID];

					if (!storage)
						continue;

					const FieldInfo* fieldInfo = storage->GetFieldInfo();

					if (!fieldInfo->IsWritable())
						continue;

					oldFieldValues[sceneID][entityID][fieldID] = Buffer::Copy(storage->GetValueBuffer());
				}

				ShutdownScriptEntity(entity, false);

				entity.GetComponent<ScriptComponent>().FieldIDs.clear();
			}

			entityMap.clear();
		}
		s_State->ScriptEntities.clear();

		bool loaded = LoadAppAssembly();

		for (const auto& [sceneID, entityFieldMap] : oldFieldValues)
		{
			auto scene = Scene::GetScene(sceneID);

			for (const auto& [entityID, fieldMap] : entityFieldMap)
			{
				Entity entity = scene->GetEntityWithUUID(entityID);
				InitializeScriptEntity(entity);

				const auto& sc = entity.GetComponent<ScriptComponent>();

				for (const auto& [fieldID, fieldValue] : fieldMap)
				{
					Ref<FieldStorageBase> storage = s_State->FieldMap[entityID][fieldID];

					if (!storage)
						continue;

					storage->SetValueBuffer(fieldValue);
				}
			}
		}

		GCManager::CollectGarbage();
		ANT_CORE_INFO_TAG("ScriptEngine", "Done!");

		s_State->ReloadAppAssembly = false;
		return loaded;
	}

	bool ScriptEngine::ShouldReloadAppAssembly() { return s_State->ReloadAppAssembly; }

	void ScriptEngine::UnloadAppAssembly()
	{
		ANT_PROFILE_FUNC();

		for (auto& [sceneID, entityInstances] : s_State->ScriptEntities)
		{
			auto scene = Scene::GetScene(sceneID);

			if (!scene)
				continue;

			for (auto& entityID : entityInstances)
				ShutdownScriptEntity(scene->TryGetEntityWithUUID(entityID));

			entityInstances.clear();
		}
		s_State->ScriptInstances.clear();

		ScriptCache::ClearCache();
		UnloadAssembly(s_State->AppAssemblyInfo);
		ScriptCache::CacheCoreClasses();
		GCManager::CollectGarbage();
	}

	bool ScriptEngine::LoadAppAssembly()
	{
		if (!FileSystem::Exists(Project::GetScriptModuleFilePath()))
		{
			ANT_CORE_ERROR_TAG("ScriptEngine", "Failed to load app assembly! Invalid filepath");
			ANT_CORE_ERROR_TAG("ScriptEngine", "Filepath = {}", Project::GetScriptModuleFilePath());
			return false;
		}

		if (s_State->AppAssemblyInfo->Assembly)
		{
			s_State->AppAssemblyInfo->ReferencedAssemblies.clear();
			s_State->AppAssemblyInfo->Assembly = nullptr;
			s_State->AppAssemblyInfo->AssemblyImage = nullptr;

			s_State->ReferencedAssemblies.clear();

			if (!LoadCoreAssembly())
				return false;
		}

		auto appAssembly = LoadMonoAssembly(Project::GetScriptModuleFilePath());

		if (appAssembly == nullptr)
		{
			ANT_CORE_ERROR_TAG("ScriptEngine", "Failed to load app assembly!");
			return false;
		}

		s_State->AppAssemblyInfo->FilePath = Project::GetScriptModuleFilePath();
		s_State->AppAssemblyInfo->Assembly = appAssembly;
		s_State->AppAssemblyInfo->AssemblyImage = mono_assembly_get_image(s_State->AppAssemblyInfo->Assembly);
		s_State->AppAssemblyInfo->Classes.clear();
		s_State->AppAssemblyInfo->IsCoreAssembly = false;
		s_State->AppAssemblyInfo->Metadata = GetMetadataForImage(s_State->AppAssemblyInfo->AssemblyImage);

		if (s_State->PostLoadCleanup)
		{
			mono_domain_unload(s_State->ScriptsDomain);

			if (s_State->OldCoreAssembly)
				s_State->OldCoreAssembly = nullptr;

			s_State->ScriptsDomain = s_State->NewScriptsDomain;
			s_State->NewScriptsDomain = nullptr;
			s_State->PostLoadCleanup = false;
		}

		s_State->AppAssemblyInfo->ReferencedAssemblies = GetReferencedAssembliesMetadata(s_State->AppAssemblyInfo->AssemblyImage);

		// Check that the referenced Script Core version matches the loaded script core version
		auto coreMetadataIt = std::find_if(s_State->AppAssemblyInfo->ReferencedAssemblies.begin(), s_State->AppAssemblyInfo->ReferencedAssemblies.end(), [](const AssemblyMetadata& metadata)
			{
				return metadata.Name == "Ant-ScriptCore";
			});

		if (coreMetadataIt == s_State->AppAssemblyInfo->ReferencedAssemblies.end())
		{
			ANT_CONSOLE_LOG_ERROR("C# project doesn't reference Ant-ScriptCore?");
			return false;
		}

		const auto& coreMetadata = s_State->CoreAssemblyInfo->Metadata;

		if (coreMetadataIt->MajorVersion != coreMetadata.MajorVersion || coreMetadataIt->MinorVersion != coreMetadata.MinorVersion)
		{
			ANT_CONSOLE_LOG_ERROR("C# project referencing an incompatible script core version!");
			ANT_CONSOLE_LOG_ERROR("Expected version: {0}.{1}, referenced version: {2}.{3}",
				coreMetadata.MajorVersion, coreMetadata.MinorVersion, coreMetadataIt->MajorVersion, coreMetadataIt->MinorVersion);

			return false;
		}

		LoadReferencedAssemblies(s_State->AppAssemblyInfo);

		ANT_CORE_INFO_TAG("ScriptEngine", "Successfully loaded app assembly from: {0}", s_State->AppAssemblyInfo->FilePath);

		ScriptGlue::RegisterGlue();
		ScriptCache::GenerateCacheForAssembly(s_State->AppAssemblyInfo);
		return true;
	}

	bool ScriptEngine::LoadAppAssemblyRuntime(Buffer appAssemblyData)
	{
		if (s_State->AppAssemblyInfo->Assembly)
		{
			s_State->AppAssemblyInfo->ReferencedAssemblies.clear();
			s_State->AppAssemblyInfo->Assembly = nullptr;
			s_State->AppAssemblyInfo->AssemblyImage = nullptr;

			s_State->ReferencedAssemblies.clear();

			if (!LoadCoreAssembly())
				return false;
		}

		auto appAssembly = LoadMonoAssemblyRuntime(appAssemblyData);
		if (appAssembly == nullptr)
		{
			ANT_CORE_ERROR_TAG("ScriptEngine", "Failed to load app assembly!");
			return false;
		}

		s_State->AppAssemblyInfo->FilePath = Project::GetScriptModuleFilePath();
		s_State->AppAssemblyInfo->Assembly = appAssembly;
		s_State->AppAssemblyInfo->AssemblyImage = mono_assembly_get_image(s_State->AppAssemblyInfo->Assembly);
		s_State->AppAssemblyInfo->Classes.clear();
		s_State->AppAssemblyInfo->IsCoreAssembly = false;
		s_State->AppAssemblyInfo->Metadata = GetMetadataForImage(s_State->AppAssemblyInfo->AssemblyImage);

		if (s_State->PostLoadCleanup)
		{
			mono_domain_unload(s_State->ScriptsDomain);

			if (s_State->OldCoreAssembly)
				s_State->OldCoreAssembly = nullptr;

			s_State->ScriptsDomain = s_State->NewScriptsDomain;
			s_State->NewScriptsDomain = nullptr;
			s_State->PostLoadCleanup = false;
		}

		s_State->AppAssemblyInfo->ReferencedAssemblies = GetReferencedAssembliesMetadata(s_State->AppAssemblyInfo->AssemblyImage);

		// Check that the referenced Script Core version matches the loaded script core version
		auto coreMetadataIt = std::find_if(s_State->AppAssemblyInfo->ReferencedAssemblies.begin(), s_State->AppAssemblyInfo->ReferencedAssemblies.end(), [](const AssemblyMetadata& metadata)
			{
				return metadata.Name == "Ant-ScriptCore";
			});

		if (coreMetadataIt == s_State->AppAssemblyInfo->ReferencedAssemblies.end())
		{
			ANT_CONSOLE_LOG_ERROR("C# project doesn't reference Ant-ScriptCore?");
			return false;
		}

		const auto& coreMetadata = s_State->CoreAssemblyInfo->Metadata;

		if (coreMetadataIt->MajorVersion != coreMetadata.MajorVersion || coreMetadataIt->MinorVersion != coreMetadata.MinorVersion)
		{
			ANT_CONSOLE_LOG_ERROR("C# project referencing an incompatible script core version!");
			ANT_CONSOLE_LOG_ERROR("Expected version: {0}.{1}, referenced version: {2}.{3}",
				coreMetadata.MajorVersion, coreMetadata.MinorVersion, coreMetadataIt->MajorVersion, coreMetadataIt->MinorVersion);

			return false;
		}

		LoadReferencedAssemblies(s_State->AppAssemblyInfo);

		ANT_CORE_INFO_TAG("ScriptEngine", "Successfully loaded app assembly from: {0}", s_State->AppAssemblyInfo->FilePath);

		ScriptGlue::RegisterGlue();
		ScriptCache::GenerateCacheForAssembly(s_State->AppAssemblyInfo);
		return true;
	}

	void ScriptEngine::SetSceneContext(const Ref<Scene>& scene, const Ref<SceneRenderer>& sceneRenderer)
	{
		s_State->SceneContext = scene;
		s_State->ActiveSceneRenderer = sceneRenderer;
	}

	Ref<Scene> ScriptEngine::GetSceneContext() { return s_State->SceneContext; }
	Ref<SceneRenderer> ScriptEngine::GetSceneRenderer() { return s_State->ActiveSceneRenderer; }

	void ScriptEngine::InitializeScriptEntity(Entity entity)
	{
		ANT_PROFILE_FUNC();

		if (!entity.HasComponent<ScriptComponent>())
			return;

		auto& sc = entity.GetComponent<ScriptComponent>();
		if (!IsModuleValid(sc.ScriptClassHandle))
		{
			ANT_CORE_ERROR_TAG("ScriptEngine", "Tried to initialize script entity with an invalid script!");
			return;
		}

		UUID sceneID = entity.GetSceneUUID();
		UUID entityID = entity.GetUUID();

		sc.FieldIDs.clear();

		ManagedClass* managedClass = ScriptCache::GetManagedClassByID(GetScriptClassIDFromComponent(sc));
		if (!managedClass)
			return;

		for (auto fieldID : managedClass->Fields)
		{
			FieldInfo* fieldInfo = ScriptCache::GetFieldByID(fieldID);

			if (!fieldInfo->HasFlag(FieldFlag::Public))
				continue;

			if (fieldInfo->IsArray())
				s_State->FieldMap[entityID][fieldID] = Ref<ArrayFieldStorage>::Create(fieldInfo);
			else
				s_State->FieldMap[entityID][fieldID] = Ref<FieldStorage>::Create(fieldInfo);

			sc.FieldIDs.push_back(fieldID);
		}

		if (s_State->ScriptEntities.find(sceneID) == s_State->ScriptEntities.end())
			s_State->ScriptEntities[sceneID] = std::vector<UUID>();

		s_State->ScriptEntities[sceneID].push_back(entityID);
	}

	void ScriptEngine::RuntimeInitializeScriptEntity(Entity entity)
	{
		ANT_PROFILE_FUNC();

		// NOTE(Peter): Intentional copy
		auto scriptComponent = entity.GetComponent<ScriptComponent>();

		if (scriptComponent.IsRuntimeInitialized)
			return;

		if (!IsModuleValid(scriptComponent.ScriptClassHandle))
		{
			const auto managedClass = ScriptCache::GetManagedClassByID(GetScriptClassIDFromComponent(scriptComponent));

			std::string className = "Unknown Script";
			if (managedClass != nullptr)
				className = managedClass->FullName;

			ANT_CORE_ERROR_TAG("ScriptEngine", "Tried to instantiate an entity with an invalid C# class '{0}'!", className);
			ANT_CONSOLE_LOG_ERROR("Tried to instantiate an entity with an invalid C# class '{0}'!", className);
			return;
		}

		MonoObject* runtimeInstance = CreateManagedObject(GetScriptClassIDFromComponent(scriptComponent), entity.GetUUID());
		GCHandle instanceHandle = GCManager::CreateObjectReference(runtimeInstance, false);
		entity.GetComponent<ScriptComponent>().ManagedInstance = instanceHandle;
		s_State->ScriptInstances[entity.GetUUID()] = instanceHandle;

		for (auto fieldID : scriptComponent.FieldIDs)
		{
			UUID entityID = entity.GetUUID();

			if (s_State->FieldMap.find(entityID) != s_State->FieldMap.end())
				s_State->FieldMap[entityID][fieldID]->SetRuntimeInstance(instanceHandle);
		}

		CallMethod(instanceHandle, "OnCreate");

		// NOTE: Don't use scriptComponent as a reference and modify it here
		//		 If OnCreate spawns a lot of entities we would loose our reference
		//		 to the script component...
		entity.GetComponent<ScriptComponent>().IsRuntimeInitialized = true;
	}

	void ScriptEngine::DuplicateScriptInstance(Entity entity, Entity targetEntity)
	{
		ANT_PROFILE_FUNC();

		if (!entity.HasComponent<ScriptComponent>() || !targetEntity.HasComponent<ScriptComponent>())
			return;

		const auto& srcScriptComp = entity.GetComponent<ScriptComponent>();
		auto& dstScriptComp = targetEntity.GetComponent<ScriptComponent>();

		if (srcScriptComp.ScriptClassHandle != dstScriptComp.ScriptClassHandle)
		{
			const auto srcClass = ScriptCache::GetManagedClassByID(GetScriptClassIDFromComponent(srcScriptComp));
			const auto dstClass = ScriptCache::GetManagedClassByID(GetScriptClassIDFromComponent(dstScriptComp));
			ANT_CORE_WARN_TAG("ScriptEngine", "Attempting to duplicate instance of: {0} to an instance of: {1}", srcClass->FullName, dstClass->FullName);
			return;
		}

		ShutdownScriptEntity(targetEntity);
		InitializeScriptEntity(targetEntity);

		UUID targetEntityID = targetEntity.GetUUID();
		UUID srcEntityID = entity.GetUUID();

		for (auto fieldID : srcScriptComp.FieldIDs)
		{
			if (s_State->FieldMap.find(srcEntityID) == s_State->FieldMap.end())
				break;

			if (s_State->FieldMap.at(srcEntityID).find(fieldID) == s_State->FieldMap.at(srcEntityID).end())
				continue;

			s_State->FieldMap[targetEntityID][fieldID]->CopyFrom(s_State->FieldMap[srcEntityID][fieldID]);
		}

		// NOTE: Ugly hack around prefab script entities referencing the wrong entity instances
		/*const auto& children = entity.Children();
		if (children.size() > 0)
		{
			for (auto fieldID : dstScriptComp.FieldIDs)
			{
				auto storage = ScriptCache::GetFieldStorage(fieldID);

				if (storage->GetField()->Type.NativeType != UnmanagedType::Entity)
					continue;

				for (size_t i = 0; i < children.size(); i++)
				{
					if (storage->IsArray())
					{
						auto arrayStorage = storage.As<ArrayFieldStorage>();

						for (size_t j = 0; j < arrayStorage->GetLength(dstScriptComp.ManagedInstance); j++)
						{
							UUID entityID = arrayStorage->GetValue<UUID>(dstScriptComp.ManagedInstance, j);
							if (entityID == children[i])
							{
								UUID newEntityID = targetEntity.Children()[i];
								arrayStorage->SetValue(dstScriptComp.ManagedInstance, j, newEntityID);
							}
						}
					}
					else
					{
						UUID entityID = storage.As<FieldStorage>()->GetValue<UUID>(dstScriptComp.ManagedInstance);
						if (entityID == children[i])
						{
							UUID newEntityID = targetEntity.Children()[i];
							storage.As<FieldStorage>()->SetValue(dstScriptComp.ManagedInstance, newEntityID);
						}
					}
				}
			}
		}*/

		if (s_State->SceneContext && s_State->SceneContext->IsPlaying())
			s_State->RuntimeDuplicatedScriptEntities.push(targetEntity);
	}

	void ScriptEngine::ShutdownScriptEntity(Entity entity, bool erase)
	{
		ANT_PROFILE_FUNC();

		if (!entity.HasComponent<ScriptComponent>())
			return;

		auto& sc = entity.GetComponent<ScriptComponent>();
		UUID sceneID = entity.GetSceneUUID();
		UUID entityID = entity.GetUUID();

		if (sc.IsRuntimeInitialized && sc.ManagedInstance != nullptr)
		{
			ShutdownRuntimeInstance(entity);
			sc.ManagedInstance = nullptr;
		}

		if (erase && s_State->ScriptEntities.find(sceneID) != s_State->ScriptEntities.end())
		{
			s_State->FieldMap.erase(entityID);
			sc.FieldIDs.clear();

			auto& scriptEntities = s_State->ScriptEntities.at(sceneID);
			scriptEntities.erase(std::remove(scriptEntities.begin(), scriptEntities.end(), entityID), scriptEntities.end());
		}
	}

	Ref<FieldStorageBase> ScriptEngine::GetFieldStorage(Entity entity, uint32_t fieldID)
	{
		UUID entityID = entity.GetUUID();
		if (s_State->FieldMap.find(entityID) == s_State->FieldMap.end())
			return nullptr;

		if (s_State->FieldMap[entityID].find(fieldID) == s_State->FieldMap[entityID].end())
			return nullptr;

		return s_State->FieldMap.at(entityID).at(fieldID);
	}

	void ScriptEngine::InitializeRuntimeDuplicatedEntities()
	{
		ANT_PROFILE_FUNC();

		while (s_State->RuntimeDuplicatedScriptEntities.size() > 0)
		{
			Entity& entity = s_State->RuntimeDuplicatedScriptEntities.top();

			if (!entity)
			{
				s_State->RuntimeDuplicatedScriptEntities.pop();
				continue;
			}

			RuntimeInitializeScriptEntity(entity);
			s_State->RuntimeDuplicatedScriptEntities.pop();
		}
	}

	bool ScriptEngine::IsEntityInstantiated(Entity entity, bool checkOnCreateCalled)
	{
		ANT_PROFILE_FUNC();

		if (!s_State->SceneContext || !s_State->SceneContext->IsPlaying())
			return false;

		if (!entity.HasComponent<ScriptComponent>())
			return false;

		const auto& scriptComponent = entity.GetComponent<ScriptComponent>();

		if (scriptComponent.ManagedInstance == nullptr)
			return false;

		if (checkOnCreateCalled && !scriptComponent.IsRuntimeInitialized)
			return false;

		return s_State->ScriptInstances.find(entity.GetUUID()) != s_State->ScriptInstances.end();
	}

	GCHandle ScriptEngine::GetEntityInstance(UUID entityID)
	{
		ANT_PROFILE_FUNC();

		if (s_State->ScriptInstances.find(entityID) == s_State->ScriptInstances.end())
			return nullptr;

		if (s_State->ScriptInstances.find(entityID) == s_State->ScriptInstances.end())
			return nullptr;

		return s_State->ScriptInstances.at(entityID);
	}

	const std::unordered_map<UUID, GCHandle>& ScriptEngine::GetEntityInstances() { return s_State->ScriptInstances; }

	uint32_t ScriptEngine::GetScriptClassIDFromComponent(const ScriptComponent& sc)
	{
		if (!AssetManager::IsAssetHandleValid(sc.ScriptClassHandle))
			return 0;

		if (s_State->AppAssemblyInfo == nullptr)
			return 0;

		Ref<ScriptAsset> scriptAsset = AssetManager::GetAsset<ScriptAsset>(sc.ScriptClassHandle);
		return scriptAsset->GetClassID();
	}

	bool ScriptEngine::IsModuleValid(AssetHandle scriptAssetHandle)
	{
		if (!AssetManager::IsAssetHandleValid(scriptAssetHandle))
			return false;

		if (s_State->AppAssemblyInfo == nullptr)
			return false;

		Ref<ScriptAsset> scriptAsset = AssetManager::GetAsset<ScriptAsset>(scriptAssetHandle);
		return ScriptCache::GetManagedClassByID(scriptAsset->GetClassID()) != nullptr;
	}


	MonoDomain* ScriptEngine::GetScriptDomain() { return s_State->ScriptsDomain; }

	void ScriptEngine::InitMono()
	{
		if (s_State->IsMonoInitialized)
			return;

		mono_set_dirs("mono/lib", "mono/etc");

		if (s_State->Config.EnableDebugging)
		{
			std::string portString = std::to_string(ApplicationSettings::Get().ScriptDebuggerListenPort);
			std::string debuggerAgentArguments = "--debugger-agent=transport=dt_socket,address=127.0.0.1:" + portString + ",server=y,suspend=n,loglevel=3,logfile=logs/MonoDebugger.log";

			// Enable mono soft debugger
			const char* options[2] = {
				debuggerAgentArguments.c_str(),
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)options);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		s_State->RootDomain = mono_jit_init("AntJITRuntime");
		ANT_CORE_ASSERT(s_State->RootDomain, "Unable to initialize Mono JIT");

		if (s_State->Config.EnableDebugging)
			mono_debug_domain_create(s_State->RootDomain);

		mono_thread_set_main(mono_thread_current());

		s_State->IsMonoInitialized = true;
		ANT_CORE_INFO_TAG("ScriptEngine", "Initialized Mono");
	}

	void ScriptEngine::ShutdownMono()
	{
		if (!s_State->IsMonoInitialized)
		{
			ANT_CORE_WARN_TAG("ScriptEngine", "Trying to shutdown Mono multiple times!");
			return;
		}

		s_State->ScriptsDomain = nullptr;
		mono_jit_cleanup(s_State->RootDomain);
		s_State->RootDomain = nullptr;

		s_State->IsMonoInitialized = false;
	}

	Ref<AssemblyInfo> ScriptEngine::GetCoreAssemblyInfo() { return s_State->CoreAssemblyInfo; }
	Ref<AssemblyInfo> ScriptEngine::GetAppAssemblyInfo() { return s_State->AppAssemblyInfo; }
	const ScriptEngineConfig& ScriptEngine::GetConfig() { return s_State->Config; }

	void ScriptEngine::CallMethod(MonoObject* monoObject, ManagedMethod* managedMethod, const void** parameters)
	{
		ANT_PROFILE_FUNC();

		MonoObject* exception = NULL;
		mono_runtime_invoke(managedMethod->Method, monoObject, const_cast<void**>(parameters), &exception);
		ScriptUtils::HandleException(exception);
	}

	MonoObject* ScriptEngine::CreateManagedObject(ManagedClass* managedClass)
	{
		MonoObject* monoObject = mono_object_new(s_State->ScriptsDomain, managedClass->Class);
		ANT_CORE_VERIFY(monoObject, "Failed to create MonoObject!");
		return monoObject;
	}

	void ScriptEngine::InitRuntimeObject(MonoObject* monoObject)
	{
		// NOTE: All this does is call the default parameterless constructor (which we can do manually)
		mono_runtime_object_init(monoObject);
	}

	MonoAssembly* ScriptEngine::LoadMonoAssembly(const std::filesystem::path& assemblyPath)
	{
		if (!FileSystem::Exists(assemblyPath))
			return nullptr;

		Buffer fileData = FileSystem::ReadBytes(assemblyPath);

		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), fileData.Size, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			ANT_CORE_ERROR_TAG("ScriptEngine", "Failed to open C# assembly '{0}'\n\t\tMessage: {1}", assemblyPath, errorMessage);
			return nullptr;
		}

		// Load C# debug symbols if debugging is enabled
		if (s_State->Config.EnableDebugging)
		{
			// First check if we have a .dll.pdb file
			bool loadDebugSymbols = true;
			std::filesystem::path pdbPath = assemblyPath.string() + ".pdb";
			if (!FileSystem::Exists(pdbPath))
			{
				// Otherwise try just .pdb
				pdbPath = assemblyPath;
				pdbPath.replace_extension("pdb");

				if (!FileSystem::Exists(pdbPath))
				{
					ANT_CORE_WARN_TAG("ScriptEngine", "Couldn't find .pdb file for assembly {0}, no debug symbols will be loaded.", assemblyPath.string());
					loadDebugSymbols = false;
				}
			}

			if (loadDebugSymbols)
			{
				ANT_CORE_INFO_TAG("ScriptEngine", "Loading debug symbols from '{0}'", pdbPath.string());
				Buffer buffer = FileSystem::ReadBytes(pdbPath);
				mono_debug_open_image_from_memory(image, buffer.As<mono_byte>(), buffer.Size);
				buffer.Release();
			}
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, 0);
		mono_image_close(image);
		return assembly;
	}

	MonoAssembly* ScriptEngine::LoadMonoAssemblyRuntime(Buffer assemblyData)
	{
		// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(assemblyData.As<char>(), assemblyData.Size, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			ANT_CORE_ERROR_TAG("ScriptEngine", "Failed to load C# assembly");
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from(image, "", &status);
		mono_image_close(image);
		return assembly;
	}

	bool ScriptEngine::LoadCoreAssembly()
	{
		ANT_CORE_TRACE_TAG("ScriptEngine", "Trying to load core assembly: {}", s_State->Config.CoreAssemblyPath.string());
		if (!FileSystem::Exists(s_State->Config.CoreAssemblyPath))
		{
			ANT_CORE_ERROR_TAG("ScriptEngine", "Could not load core assembly! Path: {}", s_State->Config.CoreAssemblyPath.string());
			return false;
		}

		if (s_State->ScriptsDomain)
		{
			ScriptCache::Shutdown();
			ScriptUtils::Shutdown();
			s_State->CoreAssemblyInfo->ReferencedAssemblies.clear();

			s_State->NewScriptsDomain = mono_domain_create_appdomain("AntScriptRuntime", nullptr);
			mono_domain_set(s_State->NewScriptsDomain, true);
			mono_domain_set_config(s_State->NewScriptsDomain, ".", "");
			s_State->PostLoadCleanup = true;
		}
		else
		{
			s_State->ScriptsDomain = mono_domain_create_appdomain("AntScriptRuntime", nullptr);
			mono_domain_set(s_State->ScriptsDomain, true);
			mono_domain_set_config(s_State->ScriptsDomain, ".", "");
			s_State->PostLoadCleanup = false;
		}

		s_State->OldCoreAssembly = s_State->CoreAssemblyInfo->Assembly;
		s_State->CoreAssemblyInfo->FilePath = s_State->Config.CoreAssemblyPath;
		s_State->CoreAssemblyInfo->Assembly = LoadMonoAssembly(s_State->Config.CoreAssemblyPath);

		if (s_State->CoreAssemblyInfo->Assembly == nullptr)
		{
			ANT_CORE_ERROR_TAG("ScriptEngine", "Failed to load core assembly!");
			return false;
		}

		s_State->CoreAssemblyInfo->Classes.clear();
		s_State->CoreAssemblyInfo->AssemblyImage = mono_assembly_get_image(s_State->CoreAssemblyInfo->Assembly);
		s_State->CoreAssemblyInfo->IsCoreAssembly = true;
		s_State->CoreAssemblyInfo->Metadata = GetMetadataForImage(s_State->CoreAssemblyInfo->AssemblyImage);
		s_State->CoreAssemblyInfo->ReferencedAssemblies = GetReferencedAssembliesMetadata(s_State->CoreAssemblyInfo->AssemblyImage);

		LoadReferencedAssemblies(s_State->CoreAssemblyInfo);

		ANT_CORE_INFO_TAG("ScriptEngine", "Successfully loaded core assembly from: {0}", s_State->Config.CoreAssemblyPath);

		ScriptCache::Init();
		ScriptUtils::Init();

		return true;
	}

	void ScriptEngine::UnloadAssembly(Ref<AssemblyInfo> assemblyInfo)
	{
		assemblyInfo->Classes.clear();
		assemblyInfo->Assembly = nullptr;
		assemblyInfo->AssemblyImage = nullptr;

		if (assemblyInfo->IsCoreAssembly)
			s_State->CoreAssemblyInfo = Ref<AssemblyInfo>::Create();
		else
			s_State->AppAssemblyInfo = Ref<AssemblyInfo>::Create();
	}

	void ScriptEngine::LoadReferencedAssemblies(const Ref<AssemblyInfo>& assemblyInfo)
	{
		static std::filesystem::path s_AssembliesBasePath = std::filesystem::absolute("mono") / "lib" / "mono" / "4.5";

		for (const auto& assemblyMetadata : assemblyInfo->ReferencedAssemblies)
		{
			// Ignore Ant-ScriptCore and mscorlib, since they're already loaded
			if (assemblyMetadata.Name.find("Ant-ScriptCore") != std::string::npos)
				continue;

			if (assemblyMetadata.Name.find("mscorlib") != std::string::npos)
				continue;

			std::filesystem::path assemblyPath = s_AssembliesBasePath / (fmt::format("{0}.dll", assemblyMetadata.Name));
			ANT_CORE_INFO_TAG("ScriptEngine", "Loading assembly {0} referenced by {1}", assemblyPath.filename().string(), assemblyInfo->FilePath.filename().string());

			MonoAssembly* assembly = LoadMonoAssembly(assemblyPath);

			if (assembly == nullptr)
			{
				ANT_CORE_ERROR_TAG("ScriptEngine", "Failed to load assembly {0} referenced by {1}", assemblyMetadata.Name, assemblyInfo->FilePath);
				continue;
			}

			s_State->ReferencedAssemblies[assemblyMetadata] = assembly;
		}
	}

	AssemblyMetadata ScriptEngine::GetMetadataForImage(MonoImage* image)
	{
		AssemblyMetadata metadata;

		const MonoTableInfo* t = mono_image_get_table_info(image, MONO_TABLE_ASSEMBLY);
		uint32_t cols[MONO_ASSEMBLY_SIZE];
		mono_metadata_decode_row(t, 0, cols, MONO_ASSEMBLY_SIZE);

		metadata.Name = mono_metadata_string_heap(image, cols[MONO_ASSEMBLY_NAME]);
		metadata.MajorVersion = cols[MONO_ASSEMBLY_MAJOR_VERSION] > 0 ? cols[MONO_ASSEMBLY_MAJOR_VERSION] : 1;
		metadata.MinorVersion = cols[MONO_ASSEMBLY_MINOR_VERSION];
		metadata.BuildVersion = cols[MONO_ASSEMBLY_BUILD_NUMBER];
		metadata.RevisionVersion = cols[MONO_ASSEMBLY_REV_NUMBER];

		return metadata;
	}

	std::vector<AssemblyMetadata> ScriptEngine::GetReferencedAssembliesMetadata(MonoImage* image)
	{
		const MonoTableInfo* t = mono_image_get_table_info(image, MONO_TABLE_ASSEMBLYREF);
		int rows = mono_table_info_get_rows(t);

		std::vector<AssemblyMetadata> metadata;
		for (int i = 0; i < rows; i++)
		{
			uint32_t cols[MONO_ASSEMBLYREF_SIZE];
			mono_metadata_decode_row(t, i, cols, MONO_ASSEMBLYREF_SIZE);

			auto& assemblyMetadata = metadata.emplace_back();
			assemblyMetadata.Name = mono_metadata_string_heap(image, cols[MONO_ASSEMBLYREF_NAME]);
			assemblyMetadata.MajorVersion = cols[MONO_ASSEMBLYREF_MAJOR_VERSION];
			assemblyMetadata.MinorVersion = cols[MONO_ASSEMBLYREF_MINOR_VERSION];
			assemblyMetadata.BuildVersion = cols[MONO_ASSEMBLYREF_BUILD_NUMBER];
			assemblyMetadata.RevisionVersion = cols[MONO_ASSEMBLYREF_REV_NUMBER];
		}

		return metadata;
	}

	void ScriptEngine::OnFileSystemChanged(const std::vector<FileSystemChangedEvent>& events)
	{
		if (!Project::GetActive()->GetConfig().AutomaticallyReloadAssembly)
			return;

		for (const auto& e : events)
		{
			const auto filepath = e.FilePath.string();

			if (filepath.find("Intermediates") != std::string::npos)
				continue;

			if (filepath.find(Project::GetProjectName()) == std::string::npos)
				continue;

			// Schedule assembly reload
			ReloadAppAssembly(true);
		}
	}
}
