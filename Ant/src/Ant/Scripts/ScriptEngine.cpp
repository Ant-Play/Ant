#include "antpch.h"
#include "Ant/Scripts/ScriptEngine.h"
#include "Ant/Scene/Scene.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>


#include <iostream>
#include <chrono>
#include <thread>

#include <Windows.h>
#include <winioctl.h>

#include "ScriptEngineRegistry.h"

namespace Ant {

	static MonoDomain* s_MonoDomain = nullptr;
	static std::string s_AssemblyPath;

	static ScriptModuleFieldMap s_PublicFields;

	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc);

	struct EntityScriptClass
	{
		std::string FullName;
		std::string ClassName;
		std::string NamespaceName;

		MonoClass* Class;
		MonoMethod* OnCreateMethod;
		MonoMethod* OnDestroyMethod;
		MonoMethod* OnUpdateMethod;

		void InitClassMethods(MonoImage* image)
		{
			OnCreateMethod = GetMethod(image, FullName + ":OnCreate()");
			OnUpdateMethod = GetMethod(image, FullName + ":OnUpdate(single)");
		}
	};

	struct EntityInstance
	{
		EntityScriptClass* ScriptClass;

		uint32_t Handle;
		Scene* SceneInstance;

		MonoObject* GetInstance()
		{
			return mono_gchandle_get_target(Handle);
		}
	};

	static std::unordered_map<std::string, EntityScriptClass> s_EntityClassMap;
	static std::unordered_map<uint32_t, EntityInstance> s_EntityInstanceMap;

	MonoAssembly* LoadAssemblyFromFile(const char* filepath)
	{
		if(filepath == NULL)
			return NULL;

		HANDLE file = CreateFileA(filepath, FILE_READ_ACCESS, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(file == INVALID_HANDLE_VALUE)
			return NULL;

		DWORD file_size = GetFileSize(file, NULL);
		if(file_size == INVALID_FILE_SIZE)
		{
			CloseHandle(file);
			return NULL;
		}

		void* file_data = malloc(file_size);
		if(file_data == NULL)
		{
			CloseHandle(file);
			return NULL;
		}

		DWORD read = 0;
		ReadFile(file, file_data, file_size, &read, NULL);
		if(file_size != read)
		{
			free(file_data);
			CloseHandle(file);
			return NULL;
		}

		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(reinterpret_cast<char*>(file_data), file_size, 1, &status, 0);
		if (status != MONO_IMAGE_OK)
			return NULL;

		auto assemb = mono_assembly_load_from_full(image, filepath, &status, 0);
		free(file_data);
		CloseHandle(file);
		mono_image_close(image);
		return assemb;
	}

	static void InitMono()
	{
		mono_set_assemblies_path("mono/lib");
		auto domain = mono_jit_init("Ant");

		char* name = (char*)"AntRuntime";
		s_MonoDomain = mono_domain_create_appdomain(name, nullptr);
	}

	static MonoAssembly* LoadAssembly(const std::string& filepath)
	{
		auto assembly = LoadAssemblyFromFile(filepath.c_str()); //mono_domain_assembly_open(s_MonoDomain, path.c_str());

		if(!assembly)
		{
			ANT_CORE_ERROR("Failed to load assembly: {0}", filepath);
			return NULL;
		}

		ANT_CORE_INFO("Successfully loaded assembly: {0}", filepath);
		return assembly;
	}

	static MonoImage* GetAssemblyImage(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		if(!image)
		{
			ANT_CORE_ERROR("ScriptsEngine::GetAssemblyImage::Failed to get assembly image");
			return NULL;
		}

		return image;
	}

	static MonoClass* GetClass(MonoImage* image, const EntityScriptClass& scriptClass)
	{
		MonoClass* monoClass = mono_class_from_name(image, scriptClass.NamespaceName.c_str(), scriptClass.ClassName.c_str());
		if(!monoClass)
		{
			ANT_CORE_ERROR("ScriptsEngine::GetClass::Failed to get class: {0}", scriptClass.FullName);
			return NULL;
		}

		return monoClass;
	}

	static uint32_t Instantiate(EntityScriptClass& scriptClass)
	{
		MonoObject* instance = mono_object_new(s_MonoDomain, scriptClass.Class);
		if(!instance)
		{
			ANT_CORE_ERROR("ScriptsEngine::Instantiate::Failed to create instance of class: {0}", scriptClass.FullName);
			return 0;
		}
		mono_runtime_object_init(instance);
		uint32_t handle = mono_gchandle_new(instance, false);
		return handle;
	}

	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc)
	{
		MonoMethodDesc* desc = mono_method_desc_new(methodDesc.c_str(), NULL);
		if(!desc)
		{
			ANT_CORE_ERROR("ScriptsEngine::GetMethod::Failed to create method desc: {0}", methodDesc);
			return NULL;
		}

		MonoMethod* method = mono_method_desc_search_in_image(desc, image);
		if(!method)
		{
			ANT_CORE_ERROR("ScriptsEngine::GetMethod::Failed to get method: {0}", methodDesc);
			return NULL;
		}

		return method;
	}

	static MonoObject* CallMethod(MonoObject* object, MonoMethod* method, void** params = nullptr)
	{
		MonoObject* exception = nullptr;
		MonoObject* result = mono_runtime_invoke(method, object, params, &exception);
		if(exception)
		{
			ANT_CORE_ERROR("ScriptsEngine::CallMethod::Failed to call method");
			return nullptr;
		}
		return result;
	}

	static void PrintClassMethods(MonoClass* monoClass)
	{
		MonoMethod* iter;
		void* ptr = 0;
		while ((iter = mono_class_get_methods(monoClass, &ptr)) != NULL)
		{
			ANT_CORE_INFO("--------------------------------");
			const char* name = mono_method_get_name(iter);
			MonoMethodDesc* methodDesc = mono_method_desc_from_method(iter);

			const char* paramNames = "";
			mono_method_get_param_names(iter, &paramNames);

			ANT_CORE_INFO("Name: {0}", name);
			ANT_CORE_INFO("Full name: {0}", mono_method_full_name(iter, true));
		}
	}

	static void PrintClassProperties(MonoClass* monoClass)
	{
		MonoProperty* iter;
		void* ptr = 0;
		while ((iter = mono_class_get_properties(monoClass, &ptr)) != NULL)
		{
			ANT_CORE_INFO("--------------------------------");
			const char* name = mono_property_get_name(iter);
			ANT_CORE_INFO("Name: {0}", name);
		}
	}

	static MonoAssembly* s_AppAssembly = nullptr;
	static MonoAssembly* s_CoreAssembly = nullptr;
	MonoImage* s_AppAssemblyImage = nullptr;
	MonoImage* s_CoreAssemblyImage = nullptr;

	static MonoString* GetName()
	{
		return mono_string_new(s_MonoDomain, "Hello");
	}

	static void LoadAntRuntimeAssembly(const std::string& path)
	{
		if(s_AppAssembly)
		{
			mono_domain_unload(s_MonoDomain);
			mono_assembly_close(s_AppAssembly);

			char* name = (char*)"AntRuntime";
			s_MonoDomain = mono_domain_create_appdomain(name, nullptr);
		}

		s_CoreAssembly = LoadAssembly("assets/scripts/Ant-ScriptCore.dll");
		s_CoreAssemblyImage = GetAssemblyImage(s_CoreAssembly);

		s_AppAssembly = LoadAssembly(path);
		s_AppAssemblyImage = GetAssemblyImage(s_AppAssembly);
		ScriptEngineRegistry::RegistryAll();
	}

	void ScriptEngine::Init(const std::string& assemblyPath)
	{
		s_AssemblyPath = assemblyPath;

		InitMono();

		LoadAntRuntimeAssembly(s_AssemblyPath);
	}

	void ScriptEngine::Shutdown()
	{
		// shutdown mono
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		EntityInstance& entityInstance = s_EntityInstanceMap[(uint32_t)entity.m_EntityHandle];
		if (entityInstance.ScriptClass->OnCreateMethod)
			CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnCreateMethod);
	}

	void ScriptEngine::OnUpdateEntity(uint32_t entityID, Timestep ts)
	{
		ANT_CORE_ASSERT(s_EntityInstanceMap.find(entityID) != s_EntityInstanceMap.end(), "Could not find entity in instance map!");

		auto& entity = s_EntityInstanceMap[entityID];

		if(entity.ScriptClass->OnUpdateMethod)
		{
			void* args[] = { &ts };
			CallMethod(entity.GetInstance(), entity.ScriptClass->OnUpdateMethod, args);
		}
	}

	static FieldType GetAntFieldType(MonoType* monoType)
	{
		int type = mono_type_get_type(monoType);
		switch (type)
		{
			case MONO_TYPE_R4: return FieldType::Float;
			case MONO_TYPE_I4: return FieldType::Int;
			case MONO_TYPE_U4: return FieldType::UnsignedInt;
			case MONO_TYPE_STRING: return FieldType::String;
			case MONO_TYPE_VALUETYPE:
			{
				char* name = mono_type_get_name(monoType);
				if (strcmp(name, "Ant.Vector2") == 0) return FieldType::Vec2;
				if (strcmp(name, "Ant.Vector3") == 0) return FieldType::Vec3;
				if (strcmp(name, "Ant.Vector4") == 0) return FieldType::Vec4;
			}
		}

		return FieldType::None;
	}

	void ScriptEngine::OnInitEntity(ScriptComponent& script, uint32_t entityID, uint32_t sceneID)
	{
		EntityScriptClass& scriptClass = s_EntityClassMap[script.ModuleName];
		scriptClass.FullName = script.ModuleName;
		if(script.ModuleName.find('.') != std::string::npos)
		{
			scriptClass.NamespaceName = script.ModuleName.substr(0, script.ModuleName.find_last_of('.'));
			scriptClass.ClassName = script.ModuleName.substr(script.ModuleName.find_last_of('.') + 1);
		}
		else
		{
			scriptClass.ClassName = script.ModuleName;
		}

		scriptClass.Class = GetClass(s_AppAssemblyImage, scriptClass);
		scriptClass.InitClassMethods(s_AppAssemblyImage);

		EntityInstance& entityInstance = s_EntityInstanceMap[entityID];
		entityInstance.ScriptClass = &scriptClass;
		entityInstance.Handle = Instantiate(scriptClass);

		MonoProperty* entityIDProperty = mono_class_get_property_from_name(scriptClass.Class, "EntityID");
		mono_property_get_get_method(entityIDProperty);
		MonoMethod* entityIDSetMethod = mono_property_get_set_method(entityIDProperty);
		void* params[] = { &entityID };
		CallMethod(entityInstance.GetInstance(), entityIDSetMethod, params);

		MonoProperty* sceneIDProperty = mono_class_get_property_from_name(scriptClass.Class, "SceneID");
		mono_property_get_get_method(sceneIDProperty);
		MonoMethod* sceneIDSetMethod = mono_property_get_set_method(sceneIDProperty);
		params[0] = { &sceneID };
		CallMethod(entityInstance.GetInstance(), sceneIDSetMethod, params);

		if(scriptClass.OnCreateMethod)
			CallMethod(entityInstance.GetInstance(), scriptClass.OnCreateMethod);

		// Retrieve public fields
		{
			MonoClassField* iter;
			void* ptr = 0;
			while ((iter = mono_class_get_fields(scriptClass.Class, &ptr)) != NULL)
			{
				const char* name = mono_field_get_name(iter);
				uint32_t flag = mono_field_get_flags(iter);
				if(flag & MONO_FIELD_ATTR_PUBLIC == 0)
					continue;

				MonoType* fieldType = mono_field_get_type(iter);
				FieldType antFieldType = GetAntFieldType(fieldType);


				// TODO: Attributes
				MonoCustomAttrInfo* attr = mono_custom_attrs_from_field(scriptClass.Class, iter);

				auto& publicField = s_PublicFields[script.ModuleName].emplace_back(name, antFieldType);
				publicField.m_EntityInstance = &entityInstance;
				publicField.m_MonoClassField = iter;
			}
		}
	}

	const Ant::ScriptModuleFieldMap& ScriptEngine::GetFieldMap()
	{
		return s_PublicFields;
	}

	void PublicField::SetValue_Internal(void* value) const
	{
		mono_field_set_value(m_EntityInstance->GetInstance(), m_MonoClassField, value);
	}

	void PublicField::GetValue_Internal(void* outValue) const
	{
		mono_field_get_value(m_EntityInstance->GetInstance(), m_MonoClassField, outValue);
	}
}
