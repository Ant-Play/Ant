#include "antpch.h"
#include "Ant/Scene/Entity.h"


namespace Ant {


	Entity::Entity(const std::string& name)
		: m_Name(name), m_Transform(1.0f)
	{

	}

	Entity::~Entity()
	{

	}

}
