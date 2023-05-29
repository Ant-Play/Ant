#pragma once

#include "PhysicsActorBase.h"
#include "PhysicsShapes.h"

namespace Ant{

	enum class EFalloffMode { Constant, Linear };

	class PhysicsActor : public PhysicsActorBase
	{
	public:
		PhysicsActor(Entity entity);
		~PhysicsActor();

		virtual glm::vec3 GetTranslation() const override { return PhysXUtils::FromPhysXVector(m_RigidActor->getGlobalPose().p); }
		virtual void SetTranslation(const glm::vec3& translation, const bool autowake = true) override;

		glm::quat GetRotation() const { return PhysXUtils::FromPhysXQuat(m_RigidActor->getGlobalPose().q); }
		glm::vec3 GetRotationEuler() const { return glm::eulerAngles(GetRotation()); }
		void SetRotation(const glm::quat& rotation, bool autowake = true);
		void Rotate(const glm::quat& rotation, bool autowake = true);

		void WakeUp();
		void PutToSleep();
		bool IsSleeping() const;

		float GetMass() const;
		void SetMass(float mass);

		float GetInverseMass() const;

		glm::mat4 GetCenterOfMass() const;
		glm::mat4 GetLocalCenterOfMass() const;

		void AddForce(const glm::vec3& force, ForceMode forceMode);
		void AddForceAtLocation(const glm::vec3& force, const glm::vec3& location, ForceMode forceMode);
		void AddTorque(const glm::vec3& torque, ForceMode forceMode);
		void AddRadialImpulse(const glm::vec3& origin, float radius, float strength, EFalloffMode falloff = EFalloffMode::Constant, bool velocityChange = false);

		glm::vec3 GetLinearVelocity() const;
		void SetLinearVelocity(const glm::vec3& velocity);
		glm::vec3 GetAngularVelocity() const;
		void SetAngularVelocity(const glm::vec3& velocity);

		float GetMaxLinearVelocity() const;
		void SetMaxLinearVelocity(float maxVelocity);
		float GetMaxAngularVelocity() const;
		void SetMaxAngularVelocity(float maxVelocity);

		float GetLinearDrag() const;
		void SetLinearDrag(float drag);
		float GetAngularDrag() const;
		void SetAngularDrag(float drag);

		glm::vec3 GetKinematicTargetPosition() const;
		glm::quat GetKinematicTargetRotation() const;
		glm::vec3 GetKinematicTargetRotationEuler() const;
		void SetKinematicTarget(const glm::vec3& targetPosition, const glm::quat& targetRotation) const;

		virtual bool SetSimulationData(uint32_t layerId) override;

		bool IsDynamic() const { return m_RigidBodyData.BodyType == RigidBodyComponent::Type::Dynamic; }

		bool IsKinematic() const { return IsDynamic() && m_RigidBodyData.IsKinematic; }
		void SetKinematic(bool isKinematic);

		virtual bool IsGravityEnabled() const override { return !m_RigidActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY); }
		virtual void SetGravityEnabled(const bool enableGravity) override;

		bool IsLockFlagSet(ActorLockFlag flag) const { return (uint8_t)flag & m_LockFlags; }
		void SetLockFlag(ActorLockFlag flag, bool value, bool forcewake = false);
		uint8_t GetLockFlags() const { return m_LockFlags; }

		bool IsAllRotationLocked() const { return IsLockFlagSet(ActorLockFlag::RotationX) && IsLockFlagSet(ActorLockFlag::RotationY) && IsLockFlagSet(ActorLockFlag::RotationZ); }

		const TransformComponent& GetTransform() const { return m_Entity.GetComponent<TransformComponent>(); }

		void AddCollider(BoxColliderComponent& collider);
		void AddCollider(SphereColliderComponent& collider);
		void AddCollider(CapsuleColliderComponent& collider);
		void AddCollider(MeshColliderComponent& collider);

		template<typename TShapeType>
		Ref<TShapeType> GetCollider()
		{
			for (const auto& collider : m_Colliders)
			{
				if (collider->GetType() == TShapeType::GetStaticType())
					return collider;
			}

			return nullptr;
		}

		physx::PxRigidActor& GetPhysXActor() const { return *m_RigidActor; }

		const std::vector<Ref<ColliderShape>>& GetCollisionShapes() const { return m_Colliders; }

		const physx::PxFilterData& GetFilterData() const { return m_FilterData; }

	private:
		void CreateRigidActor();
		void OnFixedUpdate(float fixedDeltaTime);
		virtual void SynchronizeTransform() override;

	private:
		RigidBodyComponent m_RigidBodyData;
		uint8_t m_LockFlags = 0;

		physx::PxRigidActor* m_RigidActor = nullptr;
		physx::PxFilterData m_FilterData;
		std::vector<Ref<ColliderShape>> m_Colliders;

	private:
		friend class PhysicsScene;
	};
}
