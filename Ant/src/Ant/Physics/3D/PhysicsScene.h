#pragma once

#include "PhysicsSettings.h"
#include "PhysicsActor.h"
#include "PhysicsController.h"
#include "PhysicsJoints.h"
#include "ContactListener.h"

#include <PhysX/PxPhysicsAPI.h>

// NOTE: Allocates roughly 800 bytes once, and then reuses
#define OVERLAP_MAX_COLLIDERS 50

namespace Ant{

	struct RaycastHit
	{
		uint64_t HitEntity = 0;
		glm::vec3 Position = glm::vec3(0.0f);
		glm::vec3 Normal = glm::vec3(0.0f);
		float Distance = 0.0f;
		Ref<ColliderShape> HitCollider = nullptr;
	};

	struct OverlapHit
	{
		Ref<PhysicsActorBase> Actor;
		Ref<ColliderShape> Shape;
	};

	class PhysicsRaycastExcludeEntityFilter;

	class PhysicsScene : public RefCounted
	{
	public:
		PhysicsScene();
		~PhysicsScene();

		void Simulate(float ts);

		Ref<PhysicsActor> GetActor(Entity entity);
		Ref<PhysicsActor> GetActor(Entity entity) const;
		Ref<PhysicsActor> CreateActor(Entity entity);
		void RemoveActor(Entity entity);
		const std::unordered_map<UUID, Ref<PhysicsActor>>& GetActors() const { return m_Actors; }

		Ref<PhysicsController> GetController(Entity entity);
		Ref<PhysicsController> CreateController(Entity entity);
		void RemoveController(Ref<PhysicsController> controller);
		const std::unordered_map<UUID, Ref<PhysicsController>>& GetControllers() const { return m_Controllers; }

		Ref<JointBase> GetJoint(Entity entity);
		Ref<JointBase> CreateJoint(Entity entity);
		void RemoveJoint(Ref<JointBase> joint);
		const std::unordered_map<UUID, Ref<JointBase>>& GetJoints() const { return m_Joints; }

		glm::vec3 GetGravity() const { return PhysXUtils::FromPhysXVector(m_PhysXScene->getGravity()); }
		void SetGravity(const glm::vec3& gravity) { m_PhysXScene->setGravity(PhysXUtils::ToPhysXVector(gravity)); }

		bool Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit);
		bool RaycastExcludeEntities(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit, const std::unordered_set<UUID>& excludedEntities);
		bool SphereCast(const glm::vec3& origin, const glm::vec3& direction, float radius, float maxDistance, RaycastHit* outHit);
		bool OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize, std::array<OverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count);
		bool OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight, std::array<OverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count);
		bool OverlapSphere(const glm::vec3& origin, float radius, std::array<OverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count);

		void AddRadialImpulse(const glm::vec3& origin, float radius, float strength, EFalloffMode falloff = EFalloffMode::Constant, bool velocityChange = false);

		bool IsValid() const { return m_PhysXScene != nullptr; }

		const Ref<Scene>& GetEntityScene() const { return m_EntityScene; }
		Ref<Scene> GetEntityScene() { return m_EntityScene; }

#ifndef ANT_DIST
		const physx::PxSimulationStatistics& GetSimulationStats() const { return m_SimulationStats; }
#endif
	private:
		void InitializeScene(const Ref<Scene>& scene);
		void Clear();

		void CreateRegions();

		bool Advance(float ts);
		void SubstepStrategy(float ts);

		bool OverlapGeometry(const glm::vec3& origin, const physx::PxGeometry& geometry, std::array<OverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count);

	private:
		Ref<Scene> m_EntityScene;
		PhysicsRaycastExcludeEntityFilter* m_ExcludeEntityFilter = nullptr;
		ContactListener m_ContactListener;

		physx::PxScene* m_PhysXScene;
		physx::PxControllerManager* m_PhysXControllerManager;

#ifndef ANT_DIST
		physx::PxSimulationStatistics m_SimulationStats;
#endif

		std::unordered_map<UUID, Ref<PhysicsActor>> m_Actors;
		std::unordered_map<UUID, Ref<PhysicsController>> m_Controllers;
		std::unordered_map<UUID, Ref<JointBase>> m_Joints;

		float m_SubStepSize;
		float m_Accumulator = 0.0f;
		uint32_t m_NumSubSteps = 0;
		const uint32_t c_MaxSubSteps = 8;

	private:
		friend class Scene;
	};
}
