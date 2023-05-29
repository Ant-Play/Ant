#pragma once

#include "Ant/Scene/Components.h"
#include "PhysicsActorBase.h"
#include <PhysX/PxPhysicsAPI.h>

#include <glm/glm.hpp>

namespace Ant {

	enum class CollisionFlags : physx::PxU8
	{
		None,
		Sides,
		Above,
		Below
	};

	class PhysicsController : public PhysicsActorBase
	{
	public:
		PhysicsController(Entity entity);
		~PhysicsController();

		virtual void SetGravityEnabled(const bool enableGravity) override;
		virtual bool IsGravityEnabled() const override { return m_HasGravity; }
		virtual bool SetSimulationData(uint32_t layerId) override;
		void SetSlopeLimit(const float slopeLimitDeg);
		void SetStepOffset(const float stepOffset);

		virtual glm::vec3 GetTranslation() const override;
		virtual void SetTranslation(const glm::vec3& translation, const bool autowake = true) override {}

		float GetSpeedDown() const;
		bool IsGrounded() const;
		CollisionFlags GetCollisionFlags() const;

		void Move(glm::vec3 displacement);
		void Jump(float jumpPower);

	private:
		void OnUpdate(float dt);
		virtual void SynchronizeTransform() override;

	private:
		physx::PxController* m_Controller = nullptr;
		physx::PxMaterial* m_Material = nullptr;
		physx::PxControllerCollisionFlags m_CollisionFlags = {};
		float m_SpeedDown = 0.0f;        // speed of controller in down direction at last update
		glm::vec3 m_Displacement = {};   // displacement (if any) for next update (comes from Move() calls)
		float m_Gravity = {};            // acceleration due to gravity (in direction opposite to controllers "up" vector)
		bool m_HasGravity = true;

	private:
		friend class PhysicsScene;
	};

}