#pragma once

#include "Ant/Scene/Entity.h"

namespace Ant {

	class PhysicsActorBase : public RefCounted
	{
	public:
		enum class Type { None = -1, Actor, Controller };
	public:
		Entity GetEntity() const { return m_Entity; }

		virtual bool SetSimulationData(uint32_t layerId) = 0;
		virtual glm::vec3 GetTranslation() const = 0;
		virtual void SetTranslation(const glm::vec3& translation, const bool autowake = true) = 0;

		virtual bool IsGravityEnabled() const = 0;
		virtual void SetGravityEnabled(const bool enableGravity) = 0;

		Type GetType() const { return m_Type; }
	private:
		virtual void SynchronizeTransform() = 0;
	protected:
		PhysicsActorBase(Type type, Entity entity)
			: m_Type(type), m_Entity(entity) {}
	protected:
		Entity m_Entity;
	private:
		Type m_Type = Type::None;

		friend class PhysicsScene;
	};
}