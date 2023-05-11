#pragma once
#include "Ant/Scene/Scene.h"
#include "Ant/Scene/Entity.h"
#include "Ant/Renderer/Mesh.h"

namespace Ant{

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void SetSelected(Entity entity);

		void OnImGuiRender();

	private:
		void DrawEntityNode(Entity entity);
		void DrawMeshNode(const Ref<Mesh>& mesh, uint32_t& imguiMeshID);
		void MeshNodeHierarchy(const Ref<Mesh>& mesh, aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f),uint32_t level = 0);
		void DrawComponent(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}