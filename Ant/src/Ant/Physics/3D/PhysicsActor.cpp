#include "antpch.h"
#include "PhysicsActor.h"
#include "PhysicsSystem.h"
#include "PhysXInternal.h"
#include "PhysicsLayer.h"

#include "Ant/Scripts/ScriptEngine.h"
#include "Ant/Scripts/ScriptUtils.h"
#include "Ant/Asset/AssetManager.h"

#include "Ant/Debug/Profiler.h"

#include <glm/gtx/compatibility.hpp>

namespace Ant{

	PhysicsActor::PhysicsActor(Entity entity)
		: PhysicsActorBase(PhysicsActorBase::Type::Actor, entity), m_RigidBodyData(entity.GetComponent<RigidBodyComponent>())
	{
		CreateRigidActor();
	}

	PhysicsActor::~PhysicsActor()
	{
		for (auto& collider : m_Colliders)
			collider->DetachFromActor(m_RigidActor);
		m_Colliders.clear();
		m_RigidActor->release();
	}

	void PhysicsActor::SetTranslation(const glm::vec3& translation, const bool autowake)
	{
		physx::PxTransform transform = m_RigidActor->getGlobalPose();
		transform.p = PhysXUtils::ToPhysXVector(translation);
		m_RigidActor->setGlobalPose(transform, autowake);

		if (m_RigidBodyData.BodyType == RigidBodyComponent::Type::Static)
			SynchronizeTransform();
	}

	void PhysicsActor::SetRotation(const glm::quat& rotation, bool autowake)
	{
		physx::PxTransform transform = m_RigidActor->getGlobalPose();
		transform.q = PhysXUtils::ToPhysXQuat(rotation);
		m_RigidActor->setGlobalPose(transform, autowake);

		if (m_RigidBodyData.BodyType == RigidBodyComponent::Type::Static)
			SynchronizeTransform();
	}

	void PhysicsActor::Rotate(const glm::quat& rotation, bool autowake)
	{
		physx::PxTransform transform = m_RigidActor->getGlobalPose();
		transform.q *= PhysXUtils::ToPhysXQuat(rotation);
		m_RigidActor->setGlobalPose(transform, autowake);

		if (m_RigidBodyData.BodyType == RigidBodyComponent::Type::Static)
			SynchronizeTransform();
	}

	void PhysicsActor::WakeUp()
	{
		if (IsDynamic())
			m_RigidActor->is<physx::PxRigidDynamic>()->wakeUp();
	}

	void PhysicsActor::PutToSleep()
	{
		if (IsDynamic())
			m_RigidActor->is<physx::PxRigidDynamic>()->putToSleep();
	}

	bool PhysicsActor::IsSleeping() const
	{
		return IsDynamic() ? m_RigidActor->is<physx::PxRigidDynamic>()->isSleeping() : false;
	}

	float PhysicsActor::GetMass() const
	{
		return !IsDynamic() ? m_RigidBodyData.Mass : m_RigidActor->is<physx::PxRigidDynamic>()->getMass();
	}

	void PhysicsActor::SetMass(float mass)
	{
		if (!IsDynamic())
			return;

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		physx::PxRigidBodyExt::setMassAndUpdateInertia(*actor, mass);
		m_Entity.GetComponent<RigidBodyComponent>().Mass = mass;
	}

	float PhysicsActor::GetInverseMass() const
	{
		return !IsDynamic() ? -m_RigidBodyData.Mass : m_RigidActor->is<physx::PxRigidDynamic>()->getInvMass();
	}

	glm::mat4 PhysicsActor::GetCenterOfMass() const
	{
		if (!IsDynamic())
			return glm::mat4(1.0f);

		const auto actor = m_RigidActor->is<physx::PxRigidDynamic>();
		return PhysXUtils::FromPhysXTransform(actor->getGlobalPose().transform(actor->getCMassLocalPose()));
	}

	glm::mat4 PhysicsActor::GetLocalCenterOfMass() const { return !IsDynamic() ? glm::mat4(1.0f) : PhysXUtils::FromPhysXTransform(m_RigidActor->is<physx::PxRigidDynamic>()->getCMassLocalPose()); }


	void PhysicsActor::AddForce(const glm::vec3& force, ForceMode forceMode)
	{
		ANT_PROFILE_FUNC();

		if (!IsDynamic() || IsKinematic())
		{
			ANT_CORE_WARN("Trying to add force to a non-dynamic or kinematic PhysicsActor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		actor->addForce(PhysXUtils::ToPhysXVector(force), (physx::PxForceMode::Enum)forceMode);
	}

	void PhysicsActor::AddForceAtLocation(const glm::vec3& force, const glm::vec3& location, ForceMode forceMode)
	{
		ANT_PROFILE_FUNC();

		if (!IsDynamic() || IsKinematic())
		{
			ANT_CORE_WARN("Trying to add force to a non-dynamic or kinematic PhysicsActor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);

		actor->wakeUp();
		physx::PxRigidBodyExt::addForceAtPos(*actor, PhysXUtils::ToPhysXVector(force), PhysXUtils::ToPhysXVector(location), (physx::PxForceMode::Enum)forceMode);
	}

	void PhysicsActor::AddTorque(const glm::vec3& torque, ForceMode forceMode)
	{
		if (!IsDynamic() || IsKinematic())
		{
			ANT_CORE_WARN("Trying to add torque to a non-dynamic or kinematic PhysicsActor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		actor->addTorque(PhysXUtils::ToPhysXVector(torque), (physx::PxForceMode::Enum)forceMode);
	}

	void PhysicsActor::AddRadialImpulse(const glm::vec3& origin, float radius, float strength, EFalloffMode falloff, bool velocityChange)
	{
		if (!IsDynamic() || IsKinematic())
			return;

		float mass = GetMass();
		glm::mat4 centerOfMassTransform = GetCenterOfMass();
		glm::vec3 centerOfMass = centerOfMassTransform[3];
		glm::vec3 delta = centerOfMass - origin;

		float magnitude = glm::length(delta);
		if (magnitude > radius)
			return;

		delta = glm::normalize(delta);

		float impulseMagnitude = strength;
		if (falloff == EFalloffMode::Linear)
			impulseMagnitude *= (1.0f - (magnitude / radius));

		glm::vec3 impulse = delta * impulseMagnitude;
		ForceMode mode = velocityChange ? ForceMode::VelocityChange : ForceMode::Impulse;
		AddForce(impulse, mode);
	}

	glm::vec3 PhysicsActor::GetLinearVelocity() const
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to get velocity of non-dynamic PhysicsActor.");
			return glm::vec3(0.0f);
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		return PhysXUtils::FromPhysXVector(actor->getLinearVelocity());
	}

	void PhysicsActor::SetLinearVelocity(const glm::vec3& velocity)
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to set velocity of non-dynamic PhysicsActor.");
			return;
		}

		m_RigidActor->is<physx::PxRigidDynamic>()->setLinearVelocity(PhysXUtils::ToPhysXVector(velocity));
	}

	glm::vec3 PhysicsActor::GetAngularVelocity() const
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to get angular velocity of non-dynamic PhysicsActor.");
			return glm::vec3(0.0f);
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		return PhysXUtils::FromPhysXVector(actor->getAngularVelocity());
	}

	void PhysicsActor::SetAngularVelocity(const glm::vec3& velocity)
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to set angular velocity of non-dynamic PhysicsActor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		actor->setAngularVelocity(PhysXUtils::ToPhysXVector(velocity));
	}

	float PhysicsActor::GetMaxLinearVelocity() const
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to get max linear velocity of non-dynamic PhysicsActor.");
			return 0.0f;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		return actor->getMaxLinearVelocity();
	}

	void PhysicsActor::SetMaxLinearVelocity(float maxVelocity)
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to set max linear velocity of non-dynamic PhysicsActor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		actor->setMaxLinearVelocity(maxVelocity);
	}

	float PhysicsActor::GetMaxAngularVelocity() const
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to get max angular velocity of non-dynamic PhysicsActor.");
			return 0.0f;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		return actor->getMaxAngularVelocity();
	}

	void PhysicsActor::SetMaxAngularVelocity(float maxVelocity)
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to set max angular velocity of non-dynamic PhysicsActor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		actor->setMaxAngularVelocity(maxVelocity);
	}

	float PhysicsActor::GetLinearDrag() const
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to get linear drag of non-dynamic PhysicsActor.");
			return 0.0f;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		return actor->getLinearDamping();
	}

	void PhysicsActor::SetLinearDrag(float drag)
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to set linear drag of non-dynamic PhysicsActor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		actor->setLinearDamping(drag);
		m_Entity.GetComponent<RigidBodyComponent>().LinearDrag = drag;
	}

	float PhysicsActor::GetAngularDrag() const
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to get angular drag of non-dynamic PhysicsActor.");
			return 0.0f;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		return actor->getAngularDamping();
	}

	void PhysicsActor::SetAngularDrag(float drag)
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Trying to set angular drag of non-dynamic PhysicsActor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		actor->setAngularDamping(drag);
		m_Entity.GetComponent<RigidBodyComponent>().AngularDrag = drag;
	}

	glm::vec3 PhysicsActor::GetKinematicTargetPosition() const
	{
		if (!IsKinematic())
		{
			ANT_CORE_WARN("Trying to get kinematic target for a non-kinematic actor.");
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		physx::PxTransform target;
		ANT_CORE_VERIFY(actor->getKinematicTarget(target), "kinematic target not set");
		return PhysXUtils::FromPhysXVector(target.p);
	}

	glm::quat PhysicsActor::GetKinematicTargetRotation() const
	{
		if (!IsKinematic())
		{
			ANT_CORE_WARN("Trying to get kinematic target for a non-kinematic actor.");
			return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		physx::PxTransform target;
		ANT_CORE_VERIFY(actor->getKinematicTarget(target), "kinematic target not set");
		return PhysXUtils::FromPhysXQuat(target.q);
	}

	glm::vec3 PhysicsActor::GetKinematicTargetRotationEuler() const
	{
		return glm::eulerAngles(GetKinematicTargetRotation());
	}

	void PhysicsActor::SetKinematicTarget(const glm::vec3& targetPosition, const glm::quat& targetRotation) const
	{
		if (!IsKinematic())
		{
			ANT_CORE_WARN("Trying to set kinematic target for a non-kinematic actor.");
			return;
		}

		physx::PxRigidDynamic* actor = m_RigidActor->is<physx::PxRigidDynamic>();
		ANT_CORE_ASSERT(actor);
		actor->setKinematicTarget(PhysXUtils::ToPhysXTransform(targetPosition, targetRotation));
	}

	bool PhysicsActor::SetSimulationData(uint32_t layerId)
	{
		for (auto& collider : m_Colliders)
		{
			if (collider->IsShared())
				return false;
		}

		const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(layerId);

		if (layerInfo.CollidesWith == 0)
			return false;

		m_FilterData = PhysXUtils::BuildFilterData(layerInfo, m_RigidBodyData.CollisionDetection);

		for (auto& collider : m_Colliders)
			collider->SetFilterData(m_FilterData);

		return true;
	}

	void PhysicsActor::SetKinematic(bool isKinematic)
	{
		if (!IsDynamic())
		{
			ANT_CORE_WARN("Static PhysicsActor can't be kinematic.");
			return;
		}

		m_RigidBodyData.IsKinematic = isKinematic;
		m_RigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, isKinematic);
		m_Entity.GetComponent<RigidBodyComponent>().IsKinematic = isKinematic;
	}

	void PhysicsActor::SetGravityEnabled(const bool enableGravity)
	{
		m_RigidActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !enableGravity);
		m_Entity.GetComponent<RigidBodyComponent>().DisableGravity = !enableGravity;
	}

	void PhysicsActor::SetLockFlag(ActorLockFlag flag, bool value, bool forcewake)
	{
		if (!IsDynamic())
			return;

		if (value)
			m_LockFlags |= (uint8_t)flag;
		else
			m_LockFlags &= ~(uint8_t)flag;

		m_RigidActor->is<physx::PxRigidDynamic>()->setRigidDynamicLockFlags((physx::PxRigidDynamicLockFlags)m_LockFlags);

		auto& component = m_Entity.GetComponent<RigidBodyComponent>();
		component.LockFlags = m_LockFlags;

		if (forcewake)
			m_RigidActor->is<physx::PxRigidDynamic>()->wakeUp();
	}

	void PhysicsActor::OnFixedUpdate(float fixedDeltaTime)
	{
		if (!ScriptEngine::IsEntityInstantiated(m_Entity))
			return;

		ScriptEngine::CallMethod(m_Entity.GetComponent<ScriptComponent>().ManagedInstance, "OnPhysicsUpdate", fixedDeltaTime);
	}

	void PhysicsActor::AddCollider(BoxColliderComponent& collider)
	{
		m_Colliders.push_back(Ref<BoxColliderShape>::Create(collider, *this, m_Entity));
	}

	void PhysicsActor::AddCollider(SphereColliderComponent& collider)
	{
		m_Colliders.push_back(Ref<SphereColliderShape>::Create(collider, *this, m_Entity));
	}

	void PhysicsActor::AddCollider(CapsuleColliderComponent& collider)
	{
		m_Colliders.push_back(Ref<CapsuleColliderShape>::Create(collider, *this, m_Entity));
	}

	void PhysicsActor::AddCollider(MeshColliderComponent& collider)
	{
		ANT_PROFILE_FUNC();

		auto colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(collider.ColliderAsset);
		ANT_CORE_VERIFY(colliderAsset);

		auto& meshCache = PhysicsSystem::GetMeshCache();

		if (!meshCache.Exists(colliderAsset))
		{
			ANT_CORE_WARN_TAG("Physics", "Tried to add an Mesh Collider with an invalid collider asset. Please make sure your collider has been cooked.");
			return;
		}

		const CachedColliderData& colliderData = meshCache.GetMeshData(colliderAsset);

		if (colliderAsset->CollisionComplexity == ECollisionComplexity::UseComplexAsSimple && IsDynamic())
		{
			ANT_CONSOLE_LOG_ERROR("Entity '{0}' ({1}) has a dynamic RigidBodyComponent along with a Complex MeshColliderComponent! This isn't allowed.", m_Entity.Name(), m_Entity.GetUUID());
			return;
		}

		// Create and add simple collider
		if (colliderData.SimpleColliderData.Submeshes.size() > 0)
		{
			Ref<ConvexMeshShape> convexShape = Ref<ConvexMeshShape>::Create(collider, *this, m_Entity);
			if (convexShape->IsValid())
				m_Colliders.push_back(convexShape);
		}

		// Create and add complex collider
		if (colliderData.ComplexColliderData.Submeshes.size() > 0 && colliderAsset->CollisionComplexity != ECollisionComplexity::UseSimpleAsComplex)
		{
			Ref<TriangleMeshShape> triangleShape = Ref<TriangleMeshShape>::Create(collider, *this, m_Entity);
			if (triangleShape->IsValid())
				m_Colliders.push_back(triangleShape);
		}
	}

	void PhysicsActor::CreateRigidActor()
	{
		ANT_PROFILE_FUNC();

		auto& sdk = PhysXInternal::GetPhysXSDK();

		Ref<Scene> scene = Scene::GetScene(m_Entity.GetSceneUUID());
		glm::mat4 transform = scene->GetWorldSpaceTransformMatrix(m_Entity);

		if (m_RigidBodyData.BodyType == RigidBodyComponent::Type::Static)
		{
			m_RigidActor = sdk.createRigidStatic(PhysXUtils::ToPhysXTransform(transform));
		}
		else
		{
			const PhysicsSettings& settings = PhysicsSystem::GetSettings();

			m_RigidActor = sdk.createRigidDynamic(PhysXUtils::ToPhysXTransform(transform));

			SetLinearDrag(m_RigidBodyData.LinearDrag);
			SetAngularDrag(m_RigidBodyData.AngularDrag);
			SetKinematic(m_RigidBodyData.IsKinematic);
			SetGravityEnabled(!m_RigidBodyData.DisableGravity);

			m_LockFlags = m_RigidBodyData.LockFlags;
			m_RigidActor->is<physx::PxRigidDynamic>()->setRigidDynamicLockFlags((physx::PxRigidDynamicLockFlags)m_LockFlags);
			m_RigidActor->is<physx::PxRigidDynamic>()->setSolverIterationCounts(settings.SolverIterations, settings.SolverVelocityIterations);
			m_RigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, m_RigidBodyData.CollisionDetection == CollisionDetectionType::Continuous);
			m_RigidActor->is<physx::PxRigidDynamic>()->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, m_RigidBodyData.CollisionDetection == CollisionDetectionType::ContinuousSpeculative);
		}

		if (!PhysicsLayerManager::IsLayerValid(m_RigidBodyData.LayerID))
		{
			m_RigidBodyData.LayerID = 0;
			m_Entity.GetComponent<RigidBodyComponent>().LayerID = 0;
		}

		const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(m_RigidBodyData.LayerID);
		m_FilterData = PhysXUtils::BuildFilterData(layerInfo, m_RigidBodyData.CollisionDetection);

		if (m_Entity.HasComponent<BoxColliderComponent>()) AddCollider(m_Entity.GetComponent<BoxColliderComponent>());
		if (m_Entity.HasComponent<SphereColliderComponent>()) AddCollider(m_Entity.GetComponent<SphereColliderComponent>());
		if (m_Entity.HasComponent<CapsuleColliderComponent>()) AddCollider(m_Entity.GetComponent<CapsuleColliderComponent>());
		if (m_Entity.HasComponent<MeshColliderComponent>()) AddCollider(m_Entity.GetComponent<MeshColliderComponent>());

		SetMass(m_RigidBodyData.Mass);

		m_RigidActor->userData = this;

#ifdef ANT_DEBUG
		const auto& name = m_Entity.GetComponent<TagComponent>().Tag;
		m_RigidActor->setName(name.c_str());
#endif
	}

	void PhysicsActor::SynchronizeTransform()
	{
		TransformComponent& transform = m_Entity.Transform();
		glm::vec3 scale = transform.Scale;
		physx::PxTransform actorPose = m_RigidActor->getGlobalPose();
		transform.Translation = PhysXUtils::FromPhysXVector(actorPose.p);
		if (!IsAllRotationLocked())
			transform.SetRotation(PhysXUtils::FromPhysXQuat(actorPose.q));

		auto scene = Scene::GetScene(m_Entity.GetSceneUUID());
		scene->ConvertToLocalSpace(m_Entity);
		transform.Scale = scale;
	}

}
