#include "antpch.h"
#include "Ant/Scene/Entity.h"


namespace Ant {


	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{

	}

}
