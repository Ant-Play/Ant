#pragma once

#include "Ant/Scene/Entity.h"
#include "PhysicsActor.h"

namespace physx {
	class PxRigidActor;
	class PxFixedJoint;
}

namespace Ant {

	enum class JointType
	{
		None = -1, Fixed = 0, Hinge, Spring
	};

	class JointBase : public RefCounted
	{
	public:
		virtual ~JointBase()
		{
			Release();
		}

		void SetConnectedEntity(Entity other);

		void GetBreakForceAndTorque(float& breakForce, float& breakTorque) const;
		void SetBreakForceAndTorque(float breakForce, float breakTorque);
		bool IsBroken() const;
		void Break();
		virtual bool IsBreakable() const = 0;

		bool IsPreProcessingEnabled() const;
		void SetPreProcessingEnabled(bool enabled);

		bool IsCollisionEnabled() const;
		void SetCollisionEnabled(bool enabled);

		const glm::vec3& GetLastReportedLinearForce() const { return m_LastReportedLinearForce; }
		const glm::vec3& GetLastReportedAngularForce() const { return m_LastReportedAngularForce; }

		Entity GetEntity() const { return m_Entity; }
		Entity GetConnectedEntity() const { return m_ConnectedEntity; }

		bool IsValid() const { return m_Joint != nullptr; }
		void Release();

		virtual const char* GetDebugName() const = 0;

		JointType GetType() const { return m_Type; }

	private:
		void PostSimulation();

	protected:
		JointBase(Entity entity, Entity connectedEntity, JointType type);

		physx::PxTransform GetLocalFrame(physx::PxRigidActor& attachActor);

		virtual void OnConnectedEntityChanged(Entity other) = 0;

	protected:
		Entity m_Entity, m_ConnectedEntity;
		physx::PxJoint* m_Joint = nullptr;

	private:
		JointType m_Type = JointType::None;
		glm::vec3 m_LastReportedLinearForce = glm::vec3(0.0f);
		glm::vec3 m_LastReportedAngularForce = glm::vec3(0.0f);

		friend class PhysicsScene;
	};

	class FixedJoint : public JointBase
	{
	public:
		FixedJoint(Entity entity, Entity connectedEntity);
		virtual ~FixedJoint();

		virtual bool IsBreakable() const override;

		virtual const char* GetDebugName() const override { return "FixedJoint"; }

	protected:
		virtual void OnConnectedEntityChanged(Entity other) override;
	};

}
