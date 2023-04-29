#include "antpch.h"
#include "Ant/Scene/Scene.h"
#include "Ant/Scene/Entity.h"
#include "Ant/Scene/Components.h"

#include "Ant/Renderer/Renderer2D.h"

#include <glm/glm.hpp>

namespace Ant {

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{

	}


	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	void Scene::OnUpdate(Timestep ts)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			//auto& transform = group.get<TransformComponent>(entity);
			//auto& sprite = group.get<SpriteRendererComponent>(entity);

			//auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad(transform, sprite.Color);
		}
	}

}
