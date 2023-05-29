#include "antpch.h"
#include "PhysicsScene.h"
#include "PhysicsSystem.h"
#include "PhysXInternal.h"
#include "PhysXUtils.h"
#include "Ant/Asset/AssetManager.h"
#include "Ant/Debug/Profiler.h"

#include "Ant/ImGui/ImGui.h"
#include "Ant/Debug/Profiler.h"

#include <glm/glm.hpp>

namespace Ant {

	class PhysicsRaycastExcludeEntityFilter : public physx::PxQueryFilterCallback
	{
	public:
		virtual physx::PxQueryHitType::Enum preFilter(
			const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override
		{
			Ref<PhysicsActorBase> object = (PhysicsActorBase*)actor->userData;
			UUID hitEntityID = object->GetEntity().GetUUID();

			if (ExcludedEntities)
			{
				if (ExcludedEntities->find(hitEntityID) != ExcludedEntities->end())
					return physx::PxQueryHitType::eNONE;
			}

			return physx::PxQueryHitType::eBLOCK;
		}

		// Unused
		virtual physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit) override
		{
			return physx::PxQueryHitType::eBLOCK;
		}

		const std::unordered_set<UUID>* ExcludedEntities = nullptr;
	};

	PhysicsScene::PhysicsScene()
	{
		const auto& settings = PhysicsSystem::GetSettings();
		m_SubStepSize = settings.FixedTimestep;

		physx::PxSceneDesc sceneDesc(PhysXInternal::GetPhysXSDK().getTolerancesScale());
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		sceneDesc.gravity = PhysXUtils::ToPhysXVector(settings.Gravity);
		sceneDesc.broadPhaseType = PhysXUtils::AntToPhysXBroadphaseType(settings.BroadphaseAlgorithm);
		sceneDesc.frictionType = PhysXUtils::AntToPhysXFrictionType(settings.FrictionModel);
		sceneDesc.filterShader = (physx::PxSimulationFilterShader)PhysXInternal::FilterShader;
		sceneDesc.cpuDispatcher = PhysXInternal::GetCPUDispatcher();
		sceneDesc.simulationEventCallback = &m_ContactListener;

		ANT_CORE_ASSERT(sceneDesc.isValid());

		m_PhysXScene = PhysXInternal::GetPhysXSDK().createScene(sceneDesc);
		m_PhysXControllerManager = PxCreateControllerManager(*m_PhysXScene);

		ANT_CORE_ASSERT(m_PhysXScene);

		CreateRegions();

		m_ExcludeEntityFilter = anew PhysicsRaycastExcludeEntityFilter();
	}

	PhysicsScene::~PhysicsScene()
	{
		Clear();

		m_PhysXControllerManager->release();
		m_PhysXScene->release();
		m_PhysXScene = nullptr;

		adelete m_ExcludeEntityFilter;
	}

	void PhysicsScene::Simulate(float ts)
	{
		if (m_EntityScene->IsPlaying())
		{
			// NOTE: This is technically not a fixed update. I'll revise this at some point
			for (auto& [entityID, actor] : m_Actors)
				actor->OnFixedUpdate(m_SubStepSize);
		}

		for (auto& [entityID, controller] : m_Controllers)
			controller->OnUpdate(ts);

		bool advanced = Advance(ts);
		if (advanced)
		{
			uint32_t nbActiveActors;
			physx::PxActor** activeActors = m_PhysXScene->getActiveActors(nbActiveActors);
			for (uint32_t i = 0; i < nbActiveActors; i++)
			{
				// NOTE: We can guarantee that this is a PhysicsActor, because controllers aren't included in active actors
				Ref<PhysicsActor> actor = (PhysicsActor*)activeActors[i]->userData;
				if (actor && !actor->IsSleeping())
					actor->SynchronizeTransform();
			}

			// NOTE: SynchronizeTransform for controllers HAVE to be done explicitly because controller actors aren't included in the Active Actors list
			for (auto& [entityID, controller] : m_Controllers)
				controller->SynchronizeTransform();

			for (auto& [entityID, joint] : m_Joints)
				joint->PostSimulation();
		}

#ifndef ANT_DIST
		m_PhysXScene->getSimulationStatistics(m_SimulationStats);
#endif
	}

	bool PhysicsScene::Advance(float dt)
	{
		SubstepStrategy(dt);

		for (uint32_t i = 0; i < m_NumSubSteps; i++)
		{
			m_PhysXScene->simulate(m_SubStepSize);
			m_PhysXScene->fetchResults(true);
		}

		return m_NumSubSteps != 0;
	}

	void PhysicsScene::SubstepStrategy(float ts)
	{
		if (m_Accumulator > m_SubStepSize)
			m_Accumulator = 0.0f;

		m_Accumulator += ts;
		if (m_Accumulator < m_SubStepSize)
		{
			m_NumSubSteps = 0;
			return;
		}

		m_NumSubSteps = glm::min(static_cast<uint32_t>(m_Accumulator / m_SubStepSize), c_MaxSubSteps);
		m_Accumulator -= (float)m_NumSubSteps * m_SubStepSize;
	}

	Ref<PhysicsActor> PhysicsScene::GetActor(Entity entity)
	{
		UUID entityID = entity.GetUUID();
		if (auto iter = m_Actors.find(entityID); iter != m_Actors.end())
			return iter->second;

		return nullptr;
	}

	Ref<PhysicsActor> PhysicsScene::GetActor(Entity entity) const
	{
		UUID entityID = entity.GetUUID();
		if (const auto iter = m_Actors.find(entityID); iter != m_Actors.end())
			return iter->second;

		return nullptr;
	}

	Ref<PhysicsActor> PhysicsScene::CreateActor(Entity entity)
	{
		ANT_PROFILE_FUNC();
		auto existingActor = GetActor(entity);
		if (existingActor)
			return existingActor;

		Ref<PhysicsActor> actor = Ref<PhysicsActor>::Create(entity);
		m_Actors[entity.GetUUID()] = actor;
		m_PhysXScene->addActor(*actor->m_RigidActor);
		return actor;
	}

	void PhysicsScene::RemoveActor(Entity entity)
	{
		ANT_PROFILE_FUNC();

		const auto it = m_Actors.find(entity.GetUUID());
		if (it == m_Actors.end())
		{
			ANT_CORE_ASSERT(false, "Physics actor not found in scene");
			return;
		}

		Ref<PhysicsActor> actor = it->second;

		if (!actor)
			return;

		ANT_CORE_ASSERT(actor->m_RigidActor);
		m_PhysXScene->removeActor(*actor->m_RigidActor);
		m_Actors.erase(it);
	}

	Ref<PhysicsController> PhysicsScene::GetController(Entity entity)
	{
		UUID entityID = entity.GetUUID();
		if (auto iter = m_Controllers.find(entityID); iter != m_Controllers.end())
			return iter->second;

		return nullptr;
	}

	Ref<PhysicsController> PhysicsScene::CreateController(Entity entity)
	{
		auto existingController = GetController(entity);
		if (existingController)
			return existingController;

		Ref<PhysicsController> controller = Ref<PhysicsController>::Create(entity);

		const auto& transformComponent = entity.GetComponent<TransformComponent>();
		const auto& characterControllerComponent = entity.GetComponent<CharacterControllerComponent>();

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			const auto& capsuleColliderComponent = entity.GetComponent<CapsuleColliderComponent>();

			Ref<PhysicsMaterial> mat = AssetManager::GetAsset<PhysicsMaterial>(capsuleColliderComponent.Material);
			if (!mat)
				mat = Ref<PhysicsMaterial>::Create(0.6f, 0.6f, 0.0f);
			controller->m_Material = PhysXInternal::GetPhysXSDK().createMaterial(mat->StaticFriction, mat->DynamicFriction, mat->Bounciness);

			float radiusScale = glm::max(transformComponent.Scale.x, transformComponent.Scale.z);

			physx::PxCapsuleControllerDesc desc;
			desc.position = PhysXUtils::ToPhysXExtendedVector(entity.Transform().Translation + capsuleColliderComponent.Offset); // not convinced this is correct.  (e.g. it needs to be world space, not local)
			desc.height = capsuleColliderComponent.Height * transformComponent.Scale.y;
			desc.radius = capsuleColliderComponent.Radius * radiusScale;
			desc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;  // TODO: get from component
			desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
			desc.slopeLimit = std::max(0.0f, cos(glm::radians(characterControllerComponent.SlopeLimitDeg)));
			desc.stepOffset = characterControllerComponent.StepOffset;
			desc.contactOffset = 0.01f;                                                     // TODO: get from component
			desc.material = controller->m_Material;
			desc.upDirection = { 0.0f, 1.0f, 0.0f };

			ANT_CORE_VERIFY(controller->m_Controller = m_PhysXControllerManager->createController(desc));
		}
		else if (entity.HasComponent<BoxColliderComponent>())
		{
			const auto& boxColliderComponent = entity.GetComponent<BoxColliderComponent>();

			Ref<PhysicsMaterial> mat = AssetManager::GetAsset<PhysicsMaterial>(boxColliderComponent.Material);
			if (!mat)
				mat = Ref<PhysicsMaterial>::Create(0.6f, 0.6f, 0.0f);
			controller->m_Material = PhysXInternal::GetPhysXSDK().createMaterial(mat->StaticFriction, mat->DynamicFriction, mat->Bounciness);

			physx::PxBoxControllerDesc desc;
			desc.position = PhysXUtils::ToPhysXExtendedVector(entity.Transform().Translation + boxColliderComponent.Offset); // not convinced this is correct.  (e.g. it needs to be world space, not local)
			desc.halfHeight = (boxColliderComponent.HalfSize.y * transformComponent.Scale.y);
			desc.halfSideExtent = (boxColliderComponent.HalfSize.x * transformComponent.Scale.x);
			desc.halfForwardExtent = (boxColliderComponent.HalfSize.z * transformComponent.Scale.z);
			desc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;  // TODO: get from component
			desc.slopeLimit = std::max(0.0f, cos(glm::radians(characterControllerComponent.SlopeLimitDeg)));
			desc.stepOffset = characterControllerComponent.StepOffset;
			desc.contactOffset = 0.01f;                                                     // TODO: get from component
			desc.material = controller->m_Material;
			desc.upDirection = { 0.0f, 1.0f, 0.0f };

			ANT_CORE_VERIFY(controller->m_Controller = m_PhysXControllerManager->createController(desc));
		}

		controller->m_HasGravity = !characterControllerComponent.DisableGravity;
		controller->m_Controller->getActor()->userData = controller.Raw();

		controller->SetSimulationData(entity.GetComponent<CharacterControllerComponent>().LayerID);

		m_Controllers[entity.GetUUID()] = controller;
		return controller;
	}

	void PhysicsScene::RemoveController(Ref<PhysicsController> controller)
	{
		if (!controller)
			return;

		if (const auto it = m_Controllers.find(controller->GetEntity().GetUUID()); it != m_Controllers.end())
			m_Controllers.erase(it);
		else
			ANT_CORE_ASSERT(false, "Physics controller not found in scene");
	}

	Ref<JointBase> PhysicsScene::GetJoint(Entity entity)
	{
		UUID entityID = entity.GetUUID();
		if (auto iter = m_Joints.find(entityID); iter != m_Joints.end())
			return iter->second;

		return nullptr;
	}

	Ref<JointBase> PhysicsScene::CreateJoint(Entity entity)
	{
		auto existingJoint = GetJoint(entity);
		if (existingJoint)
			return existingJoint;

		Ref<JointBase> joint = nullptr;

		const auto& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
		Entity connectedEntity = m_EntityScene->TryGetEntityWithUUID(fixedJointComponent.ConnectedEntity);

		if (!connectedEntity)
			return nullptr;

		if (entity.HasComponent<FixedJointComponent>())
			joint = Ref<FixedJoint>::Create(entity, connectedEntity);

		if (!joint || !joint->IsValid())
			return nullptr;

		m_Joints[entity.GetUUID()] = joint;
		return joint;
	}

	void PhysicsScene::RemoveJoint(Ref<JointBase> joint)
	{
		if (!joint)
			return;

		if (const auto it = m_Joints.find(joint->GetEntity().GetUUID()); it != m_Joints.end())
			m_Joints.erase(it);
		else
			ANT_CORE_ASSERT(false, "Physics joint not found in scene");
	}

	bool PhysicsScene::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit)
	{
		ANT_PROFILE_FUNC();

		physx::PxRaycastBuffer hitInfo;
		bool result = m_PhysXScene->raycast(PhysXUtils::ToPhysXVector(origin), PhysXUtils::ToPhysXVector(glm::normalize(direction)), maxDistance, hitInfo);

		if (result)
		{
			physx::PxRaycastHit& closestHit = hitInfo.block;

			Ref<PhysicsActorBase> object = (PhysicsActorBase*)closestHit.actor->userData;
			outHit->HitEntity = object->GetEntity().GetUUID();
			outHit->Position = PhysXUtils::FromPhysXVector(closestHit.position);
			outHit->Normal = PhysXUtils::FromPhysXVector(closestHit.normal);
			outHit->Distance = closestHit.distance;
			outHit->HitCollider = (ColliderShape*)(closestHit.shape->userData);
			return result;
		}

		return result;
	}

	bool PhysicsScene::RaycastExcludeEntities(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit* outHit, const std::unordered_set<UUID>& excludedEntities)
	{
		ANT_PROFILE_FUNC();

		physx::PxRaycastBuffer hitInfo;

		m_ExcludeEntityFilter->ExcludedEntities = &excludedEntities;

		physx::PxQueryFilterData queryFilterData(physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::ePREFILTER);
		bool result = m_PhysXScene->raycast(PhysXUtils::ToPhysXVector(origin), PhysXUtils::ToPhysXVector(glm::normalize(direction)), maxDistance, hitInfo,
			physx::PxHitFlags(physx::PxHitFlag::eDEFAULT), queryFilterData, m_ExcludeEntityFilter);

		m_ExcludeEntityFilter->ExcludedEntities = nullptr;

		if (result)
		{
			physx::PxRaycastHit& closestHit = hitInfo.block;

			Ref<PhysicsActorBase> object = (PhysicsActorBase*)closestHit.actor->userData;
			outHit->HitEntity = object->GetEntity().GetUUID();
			outHit->Position = PhysXUtils::FromPhysXVector(closestHit.position);
			outHit->Normal = PhysXUtils::FromPhysXVector(closestHit.normal);
			outHit->Distance = closestHit.distance;
			outHit->HitCollider = (ColliderShape*)(closestHit.shape->userData);
		}

		return result;
	}

	bool PhysicsScene::SphereCast(const glm::vec3& origin, const glm::vec3& direction, float radius, float maxDistance, RaycastHit* outHit)
	{
		ANT_PROFILE_FUNC();

		physx::PxSweepBuffer sweepBuffer;
		bool result = m_PhysXScene->sweep(physx::PxSphereGeometry(radius), physx::PxTransform(PhysXUtils::ToPhysXVector(origin)),
			PhysXUtils::ToPhysXVector(direction), maxDistance, sweepBuffer);

		if (result)
		{
			physx::PxSweepHit& closestHit = sweepBuffer.block;

			Ref<PhysicsActorBase> object = (PhysicsActorBase*)closestHit.actor->userData;
			outHit->HitEntity = object->GetEntity().GetUUID();
			outHit->Position = PhysXUtils::FromPhysXVector(closestHit.position);
			outHit->Normal = PhysXUtils::FromPhysXVector(closestHit.normal);
			outHit->Distance = closestHit.distance;
			outHit->HitCollider = (ColliderShape*)(closestHit.shape->userData);
			return result;
		}

		return result;
	}

	bool PhysicsScene::OverlapBox(const glm::vec3& origin, const glm::vec3& halfSize, std::array<OverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
	{
		return OverlapGeometry(origin, physx::PxBoxGeometry(halfSize.x, halfSize.y, halfSize.z), buffer, count);
	}

	bool PhysicsScene::OverlapCapsule(const glm::vec3& origin, float radius, float halfHeight, std::array<OverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
	{
		return OverlapGeometry(origin, physx::PxCapsuleGeometry(radius, halfHeight), buffer, count);
	}

	bool PhysicsScene::OverlapSphere(const glm::vec3& origin, float radius, std::array<OverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
	{
		return OverlapGeometry(origin, physx::PxSphereGeometry(radius), buffer, count);
	}

	void PhysicsScene::AddRadialImpulse(const glm::vec3& origin, float radius, float strength, EFalloffMode falloff, bool velocityChange)
	{
		ANT_PROFILE_FUNC();
		std::array<OverlapHit, OVERLAP_MAX_COLLIDERS> overlappedColliders;
		memset(overlappedColliders.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(OverlapHit));

		uint32_t count = 0;
		if (!OverlapSphere(origin, radius, overlappedColliders, count))
			return;

		for (uint32_t i = 0; i < count; i++)
		{
			auto actorBase = overlappedColliders[i].Actor;
			if (actorBase->GetType() == PhysicsActorBase::Type::Actor)
			{
				auto actor = actorBase.As<PhysicsActor>();
				if (actor->IsDynamic() && !actor->IsKinematic())
					actor->AddRadialImpulse(origin, radius, strength, falloff, velocityChange);
			}
		}
	}

	void PhysicsScene::InitializeScene(const Ref<Scene>& scene)
	{
		m_EntityScene = scene;

		{
			auto view = m_EntityScene->GetAllEntitiesWith<RigidBodyComponent>();

			for (auto entity : view)
			{
				Entity e = { entity, m_EntityScene.Raw() };
				CreateActor(e);
			}
		}

		{
			auto view = m_EntityScene->GetAllEntitiesWith<CharacterControllerComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, m_EntityScene.Raw() };
				CreateController(e);
			}
		}

		{
			auto view = m_EntityScene->GetAllEntitiesWith<TransformComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, m_EntityScene.Raw() };

				if (e.HasComponent<RigidBodyComponent>())
					continue;

				if (e.HasComponent<CharacterControllerComponent>())
					continue;

				if (!e.HasAny<BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, MeshColliderComponent>())
					continue;

				auto& rigidbody = e.AddComponent<RigidBodyComponent>();
				rigidbody.BodyType = RigidBodyComponent::Type::Static;
				CreateActor(e);
			}
		}

		// Joints
		{
			auto view = m_EntityScene->GetAllEntitiesWith<FixedJointComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, m_EntityScene.Raw() };
				CreateJoint(e);
			}
		}
	}

	void PhysicsScene::Clear()
	{
		ANT_CORE_ASSERT(m_PhysXScene);

		for (auto& actor : m_Actors)
		{
			ANT_CORE_ASSERT(actor.second->m_RigidActor);
			m_PhysXScene->removeActor(*actor.second->m_RigidActor);
		}

		m_Joints.clear();
		m_Controllers.clear();
		m_Actors.clear();

		SharedShapeManager::ClearSharedShapes();

		m_EntityScene = nullptr;
	}

	void PhysicsScene::CreateRegions()
	{
		const PhysicsSettings& settings = PhysicsSystem::GetSettings();

		if (settings.BroadphaseAlgorithm == BroadphaseType::AutomaticBoxPrune)
			return;

		physx::PxBounds3* regionBounds = anew physx::PxBounds3[settings.WorldBoundsSubdivisions * settings.WorldBoundsSubdivisions];
		physx::PxBounds3 globalBounds(PhysXUtils::ToPhysXVector(settings.WorldBoundsMin), PhysXUtils::ToPhysXVector(settings.WorldBoundsMax));
		uint32_t regionCount = physx::PxBroadPhaseExt::createRegionsFromWorldBounds(regionBounds, globalBounds, settings.WorldBoundsSubdivisions);

		for (uint32_t i = 0; i < regionCount; i++)
		{
			physx::PxBroadPhaseRegion region;
			region.bounds = regionBounds[i];
			m_PhysXScene->addBroadPhaseRegion(region);
		}
	}

	static std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> s_OverlapBuffer;
	bool PhysicsScene::OverlapGeometry(const glm::vec3& origin, const physx::PxGeometry& geometry, std::array<OverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
	{
		ANT_PROFILE_FUNC();

		physx::PxOverlapBuffer buf(s_OverlapBuffer.data(), OVERLAP_MAX_COLLIDERS);
		physx::PxTransform pose = PhysXUtils::ToPhysXTransform(glm::translate(glm::mat4(1.0f), origin));

		bool result = m_PhysXScene->overlap(geometry, pose, buf);
		if (result)
		{
			count = buf.nbTouches > OVERLAP_MAX_COLLIDERS ? OVERLAP_MAX_COLLIDERS : buf.nbTouches;

			for (uint32_t i = 0; i < count; i++)
			{
				buffer[i].Actor = (PhysicsActorBase*)s_OverlapBuffer[i].actor->userData;
				buffer[i].Shape = (ColliderShape*)s_OverlapBuffer[i].shape->userData;
			}
		}

		return result;
	}

}