#include "antpch.h"
#include "PhysicsSystem.h"
#include "PhysXInternal.h"
#include "Debug/PhysXDebugger.h"
#include "PhysicsLayer.h"

#include "Ant/Core/Application.h"
#include "Ant/Project/Project.h"

#include "Ant/Asset/AssetManager.h"

namespace Ant{

	void PhysicsSystem::Init()
	{
		PhysXInternal::Initialize();

		s_PhysicsMeshCache.Init();

		Application::Get().AddEventCallback(OnEvent);
	}

	void PhysicsSystem::Shutdown()
	{
		s_PhysicsMeshCache.Clear();
		PhysicsLayerManager::ClearLayers();
		PhysXInternal::Shutdown();
	}

	const std::string& PhysicsSystem::GetLastErrorMessage() { return PhysXInternal::GetLastErrorMessage(); }

	Ref<MeshColliderAsset> PhysicsSystem::GetOrCreateColliderAsset(Entity entity, MeshColliderComponent& component)
	{
		Ref<MeshColliderAsset> colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(component.ColliderAsset);

		if (colliderAsset)
			return colliderAsset;

		if (entity.HasComponent<MeshComponent>())
		{
			auto& mc = entity.GetComponent<MeshComponent>();
			component.ColliderAsset = AssetManager::CreateMemoryOnlyAsset<MeshColliderAsset>(mc.Mesh);
			component.SubmeshIndex = mc.SubmeshIndex;
		}
		else if (entity.HasComponent<StaticMeshComponent>())
		{
			component.ColliderAsset = AssetManager::CreateMemoryOnlyAsset<MeshColliderAsset>(entity.GetComponent<StaticMeshComponent>().StaticMesh);
		}

		colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(component.ColliderAsset);

		if (colliderAsset && !PhysicsSystem::GetMeshCache().Exists(colliderAsset))
			CookingFactory::CookMesh(component.ColliderAsset);

		return colliderAsset;
	}

	void PhysicsSystem::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

#ifdef ANT_DEBUG
		dispatcher.Dispatch<ScenePreStartEvent>([](ScenePreStartEvent& e)
			{
				if (s_PhysicsSettings.DebugOnPlay && !PhysXDebugger::IsDebugging())
					PhysXDebugger::StartDebugging((Project::GetActive()->GetProjectDirectory() / "PhysXDebugInfo").string(), s_PhysicsSettings.DebugType == PhysicsDebugType::LiveDebug);
				return false;
			});

		dispatcher.Dispatch<ScenePreStopEvent>([](ScenePreStopEvent& e)
			{
				if (s_PhysicsSettings.DebugOnPlay)
					PhysXDebugger::StopDebugging();
				return false;
			});
#endif
	}

}
