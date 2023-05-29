#include "antpch.h"
#include "PhysicsController.h"

#include "PhysicsSystem.h"
#include "PhysicsLayer.h"
#include "PhysXUtils.h"

namespace Ant {

	PhysicsController::PhysicsController(Entity entity)
		: PhysicsActorBase(PhysicsActorBase::Type::Controller, entity)
		, m_Gravity(glm::length(PhysicsSystem::GetSettings().Gravity))
	{
	}

	PhysicsController::~PhysicsController()
	{
		if (m_Controller)
			m_Controller->release();

		if (m_Material)
			m_Material->release();
	}

	void PhysicsController::SetGravityEnabled(const bool enableGravity)
	{
		m_HasGravity = enableGravity;
		m_Entity.GetComponent<CharacterControllerComponent>().DisableGravity = !enableGravity;
	}

	bool PhysicsController::SetSimulationData(uint32_t layerId)
	{
		const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(layerId);

		if (layerInfo.CollidesWith == 0)
			return false;

		// Hard coded to use continuous detection right now
		physx::PxFilterData filterData = PhysXUtils::BuildFilterData(layerInfo, CollisionDetectionType::Continuous);

		const auto actor = m_Controller->getActor();
		ANT_CORE_ASSERT(actor && actor->getNbShapes() == 1);
		physx::PxShape* shape;
		actor->getShapes(&shape, 1);
		shape->setSimulationFilterData(filterData);
		return true;
	}

	void PhysicsController::SetSlopeLimit(const float slopeLimitDeg)
	{
		if (m_Controller)
			m_Controller->setSlopeLimit(std::max(0.0f, cos(glm::radians(slopeLimitDeg))));
		m_Entity.GetComponent<CharacterControllerComponent>().SlopeLimitDeg = slopeLimitDeg;
	}

	void PhysicsController::SetStepOffset(const float stepOffset)
	{
		if (m_Controller)
			m_Controller->setStepOffset(stepOffset);
		m_Entity.GetComponent<CharacterControllerComponent>().StepOffset = stepOffset;
	}

	void PhysicsController::Move(glm::vec3 displacement)
	{
		m_Displacement += displacement;
	}

	void PhysicsController::Jump(float jumpPower)
	{
		m_SpeedDown = -1.0f * jumpPower;
	}

	glm::vec3 PhysicsController::GetTranslation() const
	{
		const auto& pxPos = m_Controller->getPosition();
		glm::vec3 pos = { pxPos.x, pxPos.y, pxPos.z };

		if (m_Entity.HasComponent<CapsuleColliderComponent>())
		{
			const auto& ccc = m_Entity.GetComponent<CapsuleColliderComponent>();
			pos -= ccc.Offset;
		}
		else if (m_Entity.HasComponent<BoxColliderComponent>())
		{
			const auto& bcc = m_Entity.GetComponent<BoxColliderComponent>();
			pos -= bcc.Offset;
		}

		return pos;
	}

	float PhysicsController::GetSpeedDown() const
	{
		return m_SpeedDown;
	}

	bool PhysicsController::IsGrounded() const
	{
		return m_CollisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN;
	}

	Ant::CollisionFlags PhysicsController::GetCollisionFlags() const
	{
		return static_cast<CollisionFlags>((physx::PxU8)m_CollisionFlags);
	}

	void PhysicsController::OnUpdate(float dt)
	{
		physx::PxControllerFilters filters; // TODO...

		if (m_HasGravity)
			m_SpeedDown += m_Gravity * dt;

		glm::vec3 displacement = m_Displacement - PhysXUtils::FromPhysXVector(m_Controller->getUpDirection()) * m_SpeedDown * dt;

		m_CollisionFlags = m_Controller->move(PhysXUtils::ToPhysXVector(displacement), 0.0, static_cast<physx::PxF32>(dt), filters);

		if (IsGrounded())
			m_SpeedDown = m_Gravity * 0.01f; // setting speed back to zero here would be more technically correct,
		// but a small non-zero gives better results (e.g. lessens jerkyness when walking down a slope)
		m_Displacement = {};
	}

	void PhysicsController::SynchronizeTransform()
	{
		TransformComponent& transform = m_Entity.Transform();
		transform.Translation = GetTranslation();
	}
}