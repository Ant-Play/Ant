#pragma once

#include "PhysicsSettings.h"
#include "PhysicsMeshCache.h"
#include "Ant/Core/Events/SceneEvents.h"

namespace Ant {

	class PhysicsSystem
	{
	public:
		static void Init();
		static void Shutdown();

		static PhysicsSettings& GetSettings() { return s_PhysicsSettings; }
		static PhysicsMeshCache& GetMeshCache() { return s_PhysicsMeshCache; }
		static const std::string& GetLastErrorMessage();

		static Ref<MeshColliderAsset> GetOrCreateColliderAsset(Entity entity, MeshColliderComponent& component);
	private:
		static void OnEvent(Event& event);
	private:
		inline static PhysicsSettings s_PhysicsSettings;
		inline static PhysicsMeshCache s_PhysicsMeshCache;
	};
}
