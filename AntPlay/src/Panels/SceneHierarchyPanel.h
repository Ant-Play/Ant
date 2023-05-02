#pragma once
#include "Ant/Core/Base.h"
#include "Ant/Scene/Scene.h"
#include "Ant/Scene/Entity.h"


namespace Ant {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);

		template<typename T, typename F>
		void DrawComponents(std::string tag, Entity entity, F const& f)
		{
			if (entity.HasComponent<T>())
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
				if (ImGui::Button("+", ImVec2{ 20, 20 }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}
				ImGui::PopStyleVar();

				bool removeComponent = false;
				if (ImGui::BeginPopup("ComponentSettings"))
				{
					if (ImGui::MenuItem("Remove component"))
						removeComponent = true;

					ImGui::EndPopup();
				}

				if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap, tag.c_str()))
				{
					f();
					ImGui::TreePop();
					ImGui::Separator();
				}

				if (removeComponent)
					entity.RemoveComponent<T>();
			}
		}
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};

}
