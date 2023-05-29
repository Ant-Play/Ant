#include "antpch.h"
#include "PhysicsJoints.h"
#include "PhysicsScene.h"
#include "PhysXInternal.h"
#include "Ant/Scripts/ScriptEngine.h"

#include <PhysX/extensions/PxFixedJoint.h>
#include <PhysX/extensions/PxDistanceJoint.h>

namespace Ant{

	JointBase::JointBase(Entity entity, Entity connectedEntity, JointType type)
		: m_Entity(entity), m_ConnectedEntity(connectedEntity), m_Type(type)
	{
		ANT_CORE_VERIFY(entity.HasComponent<RigidBodyComponent>() || connectedEntity.HasComponent<RigidBodyComponent>());
	}

	void JointBase::SetConnectedEntity(Entity other)
	{
		ANT_CORE_VERIFY(m_Joint);

		m_ConnectedEntity = other;

		OnConnectedEntityChanged(other);

		auto physicsScene = Scene::GetScene(m_Entity.GetSceneUUID())->GetPhysicsScene();

		auto& actor0 = physicsScene->GetActor(m_Entity)->GetPhysXActor();
		auto& actor1 = physicsScene->GetActor(other)->GetPhysXActor();

		physx::PxTransform localFrame0 = GetLocalFrame(actor0);
		physx::PxTransform localFrame1 = GetLocalFrame(actor1);

		m_Joint->setActors(&actor1, &actor0);
		m_Joint->setLocalPose(physx::PxJointActorIndex::eACTOR0, localFrame1);
		m_Joint->setLocalPose(physx::PxJointActorIndex::eACTOR1, localFrame0);
	}

	void JointBase::GetBreakForceAndTorque(float& breakForce, float& breakTorque) const
	{
		ANT_CORE_VERIFY(m_Joint);
		m_Joint->getBreakForce(breakForce, breakTorque);
	}

	void JointBase::SetBreakForceAndTorque(float breakForce, float breakTorque)
	{
		ANT_CORE_VERIFY(m_Joint);
		m_Joint->setBreakForce(breakForce, breakTorque);
	}

	bool JointBase::IsBroken() const
	{
		ANT_CORE_VERIFY(m_Joint);
		return m_Joint->getConstraintFlags() & physx::PxConstraintFlag::eBROKEN;
	}

	void JointBase::Break()
	{
		ANT_CORE_VERIFY(m_Joint);
		m_Joint->setConstraintFlag(physx::PxConstraintFlag::eBROKEN, true);
	}

	bool JointBase::IsPreProcessingEnabled() const
	{
		ANT_CORE_VERIFY(m_Joint);
		return !(m_Joint->getConstraintFlags() & physx::PxConstraintFlag::eDISABLE_PREPROCESSING);
	}

	void JointBase::SetPreProcessingEnabled(bool enabled)
	{
		ANT_CORE_VERIFY(m_Joint);
		m_Joint->setConstraintFlag(physx::PxConstraintFlag::eDISABLE_PREPROCESSING, !enabled);
	}

	bool JointBase::IsCollisionEnabled() const
	{
		ANT_CORE_VERIFY(m_Joint);
		return m_Joint->getConstraintFlags() & physx::PxConstraintFlag::eCOLLISION_ENABLED;
	}

	void JointBase::SetCollisionEnabled(bool enabled)
	{
		ANT_CORE_VERIFY(m_Joint);
		m_Joint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, enabled);
	}

	void JointBase::PostSimulation()
	{
		physx::PxVec3 linear(0.0f), angular(0.0f);
		m_Joint->getConstraint()->getForce(linear, angular);
		m_LastReportedLinearForce = PhysXUtils::FromPhysXVector(linear);
		m_LastReportedAngularForce = PhysXUtils::FromPhysXVector(angular);
	}

	physx::PxTransform JointBase::GetLocalFrame(physx::PxRigidActor& attachActor)
	{
		auto scene = Scene::GetScene(m_Entity.GetSceneUUID());

		const auto& worldTransform = scene->GetWorldSpaceTransform(m_Entity);

		glm::quat rotation = worldTransform.GetRotation();
		glm::vec3 globalNormal = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 globalAxis = rotation * glm::vec3(0.0f, 1.0f, 0.0f);

		physx::PxVec3 localAnchor = attachActor.getGlobalPose().transformInv(PhysXUtils::ToPhysXVector(worldTransform.Translation));
		physx::PxVec3 localNormal = attachActor.getGlobalPose().rotateInv(PhysXUtils::ToPhysXVector(globalNormal));
		physx::PxVec3 localAxis = attachActor.getGlobalPose().rotateInv(PhysXUtils::ToPhysXVector(globalAxis));

		physx::PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));
		physx::PxTransform localFrame;
		localFrame.p = localAnchor;
		localFrame.q = physx::PxQuat(rot);
		localFrame.q.normalize();

		return localFrame;
	}

	void JointBase::Release()
	{
		if (m_Joint != nullptr)
		{
			m_Joint->release();
			m_Joint = nullptr;
		}
	}

	FixedJoint::FixedJoint(Entity entity, Entity connectedEntity)
		: JointBase(entity, connectedEntity, JointType::Fixed)
	{
		auto physicsScene = Scene::GetScene(entity.GetSceneUUID())->GetPhysicsScene();

		const auto& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
		ANT_CORE_VERIFY(connectedEntity.GetUUID() == fixedJointComponent.ConnectedEntity);

		auto& actor0 = physicsScene->GetActor(entity)->GetPhysXActor();
		auto& actor1 = physicsScene->GetActor(connectedEntity)->GetPhysXActor();

		physx::PxTransform localFrame0 = GetLocalFrame(actor0);
		physx::PxTransform localFrame1 = GetLocalFrame(actor1);

		m_Joint = physx::PxFixedJointCreate(PhysXInternal::GetPhysXSDK(), &actor1, localFrame1, &actor0, localFrame0);
		ANT_CORE_ASSERT(m_Joint, "Failed to create FixedJoint!");

		if (fixedJointComponent.IsBreakable)
			m_Joint->setBreakForce(fixedJointComponent.BreakForce, fixedJointComponent.BreakTorque);
		else
			m_Joint->setBreakForce(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());

		m_Joint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, fixedJointComponent.EnableCollision);
		m_Joint->setConstraintFlag(physx::PxConstraintFlag::eDISABLE_PREPROCESSING, !fixedJointComponent.EnablePreProcessing);
		m_Joint->userData = this;
	}

	FixedJoint::~FixedJoint()
	{
	}

	void FixedJoint::OnConnectedEntityChanged(Entity other)
	{
		auto& fixedJointComponent = m_Entity.GetComponent<FixedJointComponent>();
		fixedJointComponent.ConnectedEntity = m_ConnectedEntity.GetUUID();
	}

	bool FixedJoint::IsBreakable() const
	{
		ANT_CORE_VERIFY(m_Joint);
		const auto& fixedJointComponent = m_Entity.GetComponent<FixedJointComponent>();
		return fixedJointComponent.IsBreakable;
	}
}