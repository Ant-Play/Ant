#include "antpch.h"
#include "SceneHierarchyPanel.h"

#include <imgui.h>
#include <imgui/imgui_internal.h>

#include "Ant/Editor/ApplicationSettings.h"
#include "Ant/Scene/Prefab.h"
#include "Ant/Renderer/Mesh.h"
#include "Ant/Scripts/ScriptEngine.h"
#include "Ant/Physics/3D/PhysicsScene.h"
#include "Ant/Physics/3D/PhysicsLayer.h"
#include "Ant/Physics/3D/CookingFactory.h"
#include "Ant/Physics/3D/PhysicsSystem.h"
#include "Ant/Audio/AudioEngine.h"
#include "Ant/Renderer/UI/Font.h"
#include "Ant/Audio/AudioComponent.h"

#include "Ant/Asset/AssetManager.h"

#include "Ant/ImGui/ImGui.h"
#include "Ant/ImGui/CustomTreeNode.h"
#include "Ant/ImGui/ImGuiWidgets.h"
#include "Ant/Renderer/Renderer.h"

#include "Ant/Core/Inputs.h"

namespace Ant {

	static ImRect s_WindowBounds;
	static bool s_ActivateSearchWidget = false;

	SelectionContext SceneHierarchyPanel::s_ActiveSelectionContext = SelectionContext::Scene;

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context, SelectionContext selectionContext, bool isWindow)
		: m_Context(context), m_SelectionContext(selectionContext), m_IsWindow(isWindow)
	{
		if (m_Context)
			m_Context->SetEntityDestroyedCallback([this](Entity entity) { OnExternalEntityDestroyed(entity); });

		m_ComponentCopyScene = Scene::CreateEmpty();
		m_ComponentCopyEntity = m_ComponentCopyScene->CreateEntity();
	}

	void SceneHierarchyPanel::SetSceneContext(const Ref<Scene>& scene)
	{
		m_Context = scene;
		if (m_Context)
			m_Context->SetEntityDestroyedCallback([this](Entity entity) { OnExternalEntityDestroyed(entity); });
	}

	void SceneHierarchyPanel::OnImGuiRender(bool& isOpen)
	{
		ANT_PROFILE_FUNC();
		if (m_IsWindow)
		{
			UI::ScopedStyle padding(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("Scene Hierarchy", &isOpen);
		}

		s_ActiveSelectionContext = m_SelectionContext;

		m_IsWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };

		{
			const float edgeOffset = 4.0f;
			UI::ShiftCursorX(edgeOffset * 3.0f);
			UI::ShiftCursorY(edgeOffset * 2.0f);

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - edgeOffset * 3.0f);

			static std::string searchedString;

			if (s_ActivateSearchWidget)
			{
				ImGui::SetKeyboardFocusHere();
				s_ActivateSearchWidget = false;
			}

			UI::Widgets::SearchWidget(searchedString);

			ImGui::Spacing();
			ImGui::Spacing();

			// Entity list
			//------------

			UI::ScopedStyle cellPadding(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 0.0f));

			// Alt row colour
			const ImU32 colRowAlt = UI::ColourWithMultipliedValue(Colours::Theme::backgroundDark, 1.3f);
			UI::ScopedColour tableBGAlt(ImGuiCol_TableRowBgAlt, colRowAlt);

			// Table
			{
				// Scrollable Table uses child window internally
				UI::ScopedColour tableBg(ImGuiCol_ChildBg, Colours::Theme::backgroundDark);

				ImGuiTableFlags tableFlags = ImGuiTableFlags_NoPadInnerX
					| ImGuiTableFlags_Resizable
					| ImGuiTableFlags_Reorderable
					| ImGuiTableFlags_ScrollY
					/*| ImGuiTableFlags_RowBg *//*| ImGuiTableFlags_Sortable*/;

				const int numColumns = 3;
				if (ImGui::BeginTable("##SceneHierarchy-Table", numColumns, tableFlags, ImVec2(ImGui::GetContentRegionAvail())))
				{

					ImGui::TableSetupColumn("Label");
					ImGui::TableSetupColumn("Type");
					ImGui::TableSetupColumn("Visibility");

					// Headers
					{
						const ImU32 colActive = UI::ColourWithMultipliedValue(Colours::Theme::groupHeader, 1.2f);
						UI::ScopedColourStack headerColours(ImGuiCol_HeaderHovered, colActive,
							ImGuiCol_HeaderActive, colActive);

						ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);

						ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f);
						for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
						{
							ImGui::TableSetColumnIndex(column);
							const char* column_name = ImGui::TableGetColumnName(column);
							UI::ScopedID columnID(column);

							UI::ShiftCursor(edgeOffset * 3.0f, edgeOffset * 2.0f);
							ImGui::TableHeader(column_name);
							UI::ShiftCursor(-edgeOffset * 3.0f, -edgeOffset * 2.0f);
						}
						ImGui::SetCursorPosX(ImGui::GetCurrentTable()->OuterRect.Min.x);
						UI::Draw::Underline(true, 0.0f, 5.0f);
					}

					// List
					{
						UI::ScopedColourStack entitySelection(ImGuiCol_Header, IM_COL32_DISABLE,
							ImGuiCol_HeaderHovered, IM_COL32_DISABLE,
							ImGuiCol_HeaderActive, IM_COL32_DISABLE);

						for (auto entity : m_Context->GetAllEntitiesWith<IDComponent, RelationshipComponent>())
						{
							Entity e(entity, m_Context.Raw());
							if (e.GetParentUUID() == 0)
								DrawEntityNode({ entity, m_Context.Raw() }, searchedString);
						}
					}

					if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
					{
						DrawEntityCreateMenu({});
						ImGui::EndPopup();
					}


					ImGui::EndTable();
				}
			}

			s_WindowBounds = ImGui::GetCurrentWindow()->Rect();
		}

		if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

			if (payload)
			{
				size_t count = payload->DataSize / sizeof(UUID);

				for (size_t i = 0; i < count; i++)
				{
					UUID entityID = *(((UUID*)payload->Data) + i);
					Entity entity = m_Context->GetEntityWithUUID(entityID);
					m_Context->UnparentEntity(entity);
				}
			}

			ImGui::EndDragDropTarget();
		}

		{
			UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(2.0, 4.0f));
			ImGui::Begin("Properties");
			m_IsHierarchyOrPropertiesFocused = m_IsWindowFocused || ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
			DrawComponents(SelectionManager::GetSelections(s_ActiveSelectionContext));
			ImGui::End();
		}

		if (m_IsWindow)
			ImGui::End();
	}

	void SceneHierarchyPanel::OnEvent(Event& event)
	{
		if (!m_IsWindowFocused)
			return;

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e)
			{
				if (ImGui::IsMouseHoveringRect(s_WindowBounds.Min, s_WindowBounds.Max, false) && !ImGui::IsAnyItemHovered())
				{
					m_FirstSelectedRow = -1;
					m_LastSelectedRow = -1;
					SelectionManager::DeselectAll();
					return true;
				}

				return false;
			});

		dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e)
			{
				if (!m_IsWindowFocused)
					return false;

				switch (e.GetKeyCode())
				{
					case KeyCode::F:
					{
						s_ActivateSearchWidget = true;
						return true;
					}
					case KeyCode::Escape:
					{
						m_FirstSelectedRow = -1;
						m_LastSelectedRow = -1;
						break;
					}
				}

				return false;
			});
	}

	void SceneHierarchyPanel::DrawEntityCreateMenu(Entity parent)
	{
		if (!ImGui::BeginMenu("Create"))
			return;

		Entity newEntity;

		if (ImGui::MenuItem("Empty Entity"))
		{
			newEntity = m_Context->CreateEntity("Empty Entity");
		}

		if (ImGui::BeginMenu("Camera"))
		{
			if (ImGui::MenuItem("From View"))
			{
				newEntity = m_Context->CreateEntity("Camera");
				newEntity.AddComponent<CameraComponent>();

				for (auto& func : m_EntityContextMenuPlugins)
					func(newEntity);
			}

			if (ImGui::MenuItem("At World Origin"))
			{
				newEntity = m_Context->CreateEntity("Camera");
				newEntity.AddComponent<CameraComponent>();
			}

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Text"))
		{
			newEntity = m_Context->CreateEntity("Text");
			auto& textComp = newEntity.AddComponent<TextComponent>();
			textComp.FontHandle = Font::GetDefaultFont()->Handle;
		}

		if (ImGui::MenuItem("Sprite"))
		{
			newEntity = m_Context->CreateEntity("Sprite");
			auto& spriteComp = newEntity.AddComponent<SpriteRendererComponent>();
		}

		if (ImGui::BeginMenu("3D"))
		{
			if (ImGui::MenuItem("Cube"))
			{
				newEntity = m_Context->CreateEntity("Cube");
				auto mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Cube.asmesh");
				if (mesh != 0)
				{
					newEntity.AddComponent<StaticMeshComponent>(mesh);
					newEntity.AddComponent<BoxColliderComponent>();
				}
				else
				{
					std::string filePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/Source";
					std::string targetFilePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/";
					if (std::filesystem::exists(filePath / std::filesystem::path("Cube.gltf")))
					{
						AssetHandle assetHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Source/Cube.gltf");
						Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
						if (asset)
						{
							Ref<StaticMesh> mesh = Project::GetEditorAssetManager()->CreateNewAsset<StaticMesh>("Cube.asmesh", targetFilePath, asset.As<MeshSource>());

							newEntity.AddComponent<StaticMeshComponent>(mesh->Handle);
							newEntity.AddComponent<BoxColliderComponent>();
						}
					}
					else
						ANT_CONSOLE_LOG_WARN("Please import the default mesh source files to the following path: {0}", filePath);
				}
			}

			if (ImGui::MenuItem("Sphere"))
			{
				newEntity = m_Context->CreateEntity("Sphere");
				auto mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Sphere.asmesh");
				if (mesh != 0)
				{
					newEntity.AddComponent<StaticMeshComponent>(mesh);
					newEntity.AddComponent<SphereColliderComponent>();
				}
				else
				{
					std::string filePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/Source";
					std::string targetFilePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/";
					if (std::filesystem::exists(filePath / std::filesystem::path("Sphere.gltf")))
					{
						AssetHandle assetHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Source/Sphere.gltf");
						Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
						if (asset)
						{
							Ref<StaticMesh> mesh = Project::GetEditorAssetManager()->CreateNewAsset<StaticMesh>("Sphere.asmesh", targetFilePath, asset.As<MeshSource>());

							newEntity.AddComponent<StaticMeshComponent>(mesh->Handle);
							newEntity.AddComponent<SphereColliderComponent>();
						}
					}
					else
						ANT_CONSOLE_LOG_WARN("Please import the default mesh source files to the following path: {0}", filePath);
				}
			}

			if (ImGui::MenuItem("Capsule"))
			{
				newEntity = m_Context->CreateEntity("Capsule");
				auto mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Capsule.asmesh");

				if (mesh != 0)
				{
					newEntity.AddComponent<StaticMeshComponent>(mesh);
					newEntity.AddComponent<CapsuleColliderComponent>();
				}
				else
				{
					std::string filePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/Source";
					std::string targetFilePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/";
					if (std::filesystem::exists(filePath / std::filesystem::path("Capsule.gltf")))
					{
						AssetHandle assetHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Source/Capsule.gltf");
						Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
						if (asset)
						{
							Ref<StaticMesh> mesh = Project::GetEditorAssetManager()->CreateNewAsset<StaticMesh>("Capsule.asmesh", targetFilePath, asset.As<MeshSource>());

							newEntity.AddComponent<StaticMeshComponent>(mesh->Handle);
							newEntity.AddComponent<CapsuleColliderComponent>();
						}
					}
					else
						ANT_CONSOLE_LOG_WARN("Please import the default mesh source files to the following path: {0}", filePath);
				}
			}

			if (ImGui::MenuItem("Cylinder"))
			{
				newEntity = m_Context->CreateEntity("Cylinder");
				auto mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Cylinder.asmesh");

				if (mesh != 0)
				{
					newEntity.AddComponent<StaticMeshComponent>(mesh);
					newEntity.AddComponent<MeshColliderComponent>();
					PhysicsSystem::GetOrCreateColliderAsset(newEntity, newEntity.GetComponent<MeshColliderComponent>());
				}
				else
				{
					std::string filePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/Source";
					std::string targetFilePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/";
					if (std::filesystem::exists(filePath / std::filesystem::path("Cylinder.gltf")))
					{
						AssetHandle assetHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Source/Cylinder.gltf");
						Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
						if (asset)
						{
							Ref<StaticMesh> mesh = Project::GetEditorAssetManager()->CreateNewAsset<StaticMesh>("Cylinder.asmesh", targetFilePath, asset.As<MeshSource>());

							newEntity.AddComponent<StaticMeshComponent>(mesh->Handle);
							newEntity.AddComponent<MeshColliderComponent>();
							PhysicsSystem::GetOrCreateColliderAsset(newEntity, newEntity.GetComponent<MeshColliderComponent>());
						}
					}
					else
						ANT_CONSOLE_LOG_WARN("Please import the default mesh source files to the following path: {0}", filePath);
				}
			}

			if (ImGui::MenuItem("Torus"))
			{
				newEntity = m_Context->CreateEntity("Torus");
				auto mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Torus.asmesh");

				if (mesh != 0)
				{
					newEntity.AddComponent<StaticMeshComponent>(mesh);
					newEntity.AddComponent<MeshColliderComponent>();
					PhysicsSystem::GetOrCreateColliderAsset(newEntity, newEntity.GetComponent<MeshColliderComponent>());
				}
				else
				{
					std::string filePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/Source";
					std::string targetFilePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/";
					if (std::filesystem::exists(filePath / std::filesystem::path("Torus.gltf")))
					{
						AssetHandle assetHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Source/Torus.gltf");
						Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
						if (asset)
						{
							Ref<StaticMesh> mesh = Project::GetEditorAssetManager()->CreateNewAsset<StaticMesh>("Torus.asmesh", targetFilePath, asset.As<MeshSource>());

							newEntity.AddComponent<StaticMeshComponent>(mesh->Handle);
							newEntity.AddComponent<MeshColliderComponent>();
							PhysicsSystem::GetOrCreateColliderAsset(newEntity, newEntity.GetComponent<MeshColliderComponent>());
						}
					}
					else
						ANT_CONSOLE_LOG_WARN("Please import the default mesh source files to the following path: {0}", filePath);
				}
			}

			if (ImGui::MenuItem("Plane"))
			{
				newEntity = m_Context->CreateEntity("Plane");
				auto mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Plane.asmesh");

				if (mesh != 0)
				{
					newEntity.AddComponent<StaticMeshComponent>(mesh);
					newEntity.AddComponent<MeshColliderComponent>();
					PhysicsSystem::GetOrCreateColliderAsset(newEntity, newEntity.GetComponent<MeshColliderComponent>());
				}
				else
				{
					std::string filePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/Source";
					std::string targetFilePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/";
					if (std::filesystem::exists(filePath / std::filesystem::path("Plane.gltf")))
					{
						AssetHandle assetHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Source/Plane.gltf");
						Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
						if (asset)
						{
							Ref<StaticMesh> mesh = Project::GetEditorAssetManager()->CreateNewAsset<StaticMesh>("Plane.asmesh", targetFilePath, asset.As<MeshSource>());

							newEntity.AddComponent<StaticMeshComponent>(mesh->Handle);
							newEntity.AddComponent<MeshColliderComponent>();
							PhysicsSystem::GetOrCreateColliderAsset(newEntity, newEntity.GetComponent<MeshColliderComponent>());
						}
					}
					else
						ANT_CONSOLE_LOG_WARN("Please import the default mesh source files to the following path: {0}", filePath);
				}
			}

			if (ImGui::MenuItem("Cone"))
			{
				newEntity = m_Context->CreateEntity("Cone");
				auto mesh = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Cone.asmesh");

				if (mesh != 0)
				{
					newEntity.AddComponent<StaticMeshComponent>(mesh);
					newEntity.AddComponent<MeshColliderComponent>();
					PhysicsSystem::GetOrCreateColliderAsset(newEntity, newEntity.GetComponent<MeshColliderComponent>());
				}
				else
				{
					std::string filePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/Source";
					std::string targetFilePath = Project::GetProjectDirectory().string() + "/Assets/Meshes/Default/";
					if (std::filesystem::exists(filePath / std::filesystem::path("Cone.gltf")))
					{
						AssetHandle assetHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilePath("Meshes/Default/Source/Cone.gltf");
						Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
						if (asset)
						{
							Ref<StaticMesh> mesh = Project::GetEditorAssetManager()->CreateNewAsset<StaticMesh>("Cone.asmesh", targetFilePath, asset.As<MeshSource>());

							newEntity.AddComponent<StaticMeshComponent>(mesh->Handle);
							newEntity.AddComponent<MeshColliderComponent>();
							PhysicsSystem::GetOrCreateColliderAsset(newEntity, newEntity.GetComponent<MeshColliderComponent>());
						}
					}
					else
						ANT_CONSOLE_LOG_WARN("Please import the default mesh source files to the following path: {0}", filePath);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Directional Light"))
		{
			newEntity = m_Context->CreateEntity("Directional Light");
			newEntity.AddComponent<DirectionalLightComponent>();
			newEntity.GetComponent<TransformComponent>().SetRotationEuler(glm::radians(glm::vec3{ 80.0f, 10.0f, 0.0f }));
		}

		if (ImGui::MenuItem("Point Light"))
		{
			newEntity = m_Context->CreateEntity("Point Light");
			newEntity.AddComponent<PointLightComponent>();
		}

		if (ImGui::MenuItem("Spot Light"))
		{
			newEntity = m_Context->CreateEntity("Spot Light");
			newEntity.AddComponent<SpotLightComponent>();
			newEntity.GetComponent<TransformComponent>().Translation = glm::vec3{ 0 };
			newEntity.GetComponent<TransformComponent>().SetRotationEuler(glm::radians(glm::vec3{ 90.0f, 0.0f, 0.0f }));
		}

		if (ImGui::MenuItem("Sky Light"))
		{
			newEntity = m_Context->CreateEntity("Sky Light");
			newEntity.AddComponent<SkyLightComponent>();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Ambient Sound"))
		{
			newEntity = m_Context->CreateEntity("Ambient Sound");
			newEntity.AddComponent<AudioComponent>();
		}

		if (newEntity)
		{
			if (parent)
			{
				m_Context->ParentEntity(newEntity, parent);
				newEntity.Transform().Translation = glm::vec3(0.0f);
			}

			SelectionManager::DeselectAll();
			SelectionManager::Select(s_ActiveSelectionContext, newEntity.GetUUID());
		}

		ImGui::EndMenu();
	}

	bool SceneHierarchyPanel::TagSearchRecursive(Entity entity, std::string_view searchFilter, uint32_t maxSearchDepth, uint32_t currentDepth)
	{
		if (searchFilter.empty())
			return false;

		for (auto child : entity.Children())
		{
			Entity e = m_Context->GetEntityWithUUID(child);
			if (e.HasComponent<TagComponent>())
			{
				if (UI::IsMatchingSearch(e.GetComponent<TagComponent>().Tag, searchFilter))
					return true;
			}

			bool found = TagSearchRecursive(e, searchFilter, maxSearchDepth, currentDepth + 1);
			if (found)
				return true;
		}
		return false;
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity, const std::string& searchFilter)
	{
		const char* name = "Unnamed Entity";
		if (entity.HasComponent<TagComponent>())
			name = entity.GetComponent<TagComponent>().Tag.c_str();

		const uint32_t maxSearchDepth = 10;
		bool hasChildMatchingSearch = TagSearchRecursive(entity, searchFilter, maxSearchDepth);

		if (!UI::IsMatchingSearch(name, searchFilter) && !hasChildMatchingSearch)
			return;

		const float edgeOffset = 4.0f;
		const float rowHeight = 21.0f;

		// ImGui item height tweaks
		auto* window = ImGui::GetCurrentWindow();
		window->DC.CurrLineSize.y = rowHeight;
		//---------------------------------------------
		ImGui::TableNextRow(0, rowHeight);

		// Label column
		//-------------

		ImGui::TableNextColumn();

		window->DC.CurrLineTextBaseOffset = 3.0f;

		const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
		const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x - 20,
									rowAreaMin.y + rowHeight };

		const bool isSelected = SelectionManager::IsSelected(s_ActiveSelectionContext, entity.GetUUID());

		ImGuiTreeNodeFlags flags = (isSelected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		if (hasChildMatchingSearch)
			flags |= ImGuiTreeNodeFlags_DefaultOpen;

		if (entity.Children().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;


		const std::string strID = fmt::format("{0}{1}", name, (uint64_t)entity.GetUUID());

		ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
		bool isRowHovered, held;
		bool isRowClicked = ImGui::ButtonBehavior(ImRect(rowAreaMin, rowAreaMax), ImGui::GetID(strID.c_str()),
			&isRowHovered, &held, ImGuiButtonFlags_AllowItemOverlap | ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
		bool wasRowRightClicked = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

		ImGui::SetItemAllowOverlap();

		ImGui::PopClipRect();

		const bool isWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		// Row colouring
		//--------------

		// Fill with light selection colour if any of the child entities selected
		auto isAnyDescendantSelected = [&](Entity ent, auto isAnyDescendantSelected) -> bool
		{
			if (SelectionManager::IsSelected(s_ActiveSelectionContext, ent.GetUUID()))
				return true;

			if (!ent.Children().empty())
			{
				for (auto& childEntityID : ent.Children())
				{
					Entity childEntity = m_Context->GetEntityWithUUID(childEntityID);
					if (isAnyDescendantSelected(childEntity, isAnyDescendantSelected))
						return true;
				}
			}

			return false;
		};

		auto fillRowWithColour = [](const ImColor& colour)
		{
			for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, colour, column);
		};

		if (isSelected)
		{
			if (isWindowFocused || UI::NavigatedTo())
				fillRowWithColour(Colours::Theme::selection);
			else
			{
				const ImColor col = UI::ColourWithMultipliedValue(Colours::Theme::selection, 0.9f);
				fillRowWithColour(UI::ColourWithMultipliedSaturation(col, 0.7f));
			}
		}
		else if (isRowHovered)
		{
			fillRowWithColour(Colours::Theme::groupHeader);
		}
		else if (isAnyDescendantSelected(entity, isAnyDescendantSelected))
		{
			fillRowWithColour(Colours::Theme::selectionMuted);
		}

		// Text colouring
		//---------------

		if (isSelected)
			ImGui::PushStyleColor(ImGuiCol_Text, Colours::Theme::backgroundDark);

		const bool missingMesh = entity.HasComponent<MeshComponent>() && (AssetManager::IsAssetHandleValid(entity.GetComponent<MeshComponent>().Mesh)
			&& AssetManager::GetAsset<Mesh>(entity.GetComponent<MeshComponent>().Mesh) && AssetManager::GetAsset<Mesh>(entity.GetComponent<MeshComponent>().Mesh)->IsFlagSet(AssetFlag::Missing));

		const bool missingStaticMesh = entity.HasComponent<StaticMeshComponent>() && (AssetManager::IsAssetHandleValid(entity.GetComponent<StaticMeshComponent>().StaticMesh)
			&& AssetManager::GetAsset<StaticMesh>(entity.GetComponent<StaticMeshComponent>().StaticMesh) && AssetManager::GetAsset<StaticMesh>(entity.GetComponent<StaticMeshComponent>().StaticMesh)->IsFlagSet(AssetFlag::Missing));

		if (missingMesh || missingStaticMesh)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.4f, 0.3f, 1.0f));

		bool isPrefab = entity.HasComponent<PrefabComponent>();
		bool isValidPrefab = false;
		if (isPrefab)
			isValidPrefab = AssetManager::IsAssetHandleValid(entity.GetComponent<PrefabComponent>().PrefabID);

		if (isPrefab && !isSelected)
			ImGui::PushStyleColor(ImGuiCol_Text, isValidPrefab ? ImVec4(0.32f, 0.7f, 0.87f, 1.0f) : ImVec4(0.87f, 0.17f, 0.17f, 1.0f));

		// Tree node
		//----------
		// TODO: clean up this mess
		ImGuiContext& g = *GImGui;
		auto& style = ImGui::GetStyle();
		const ImVec2 label_size = ImGui::CalcTextSize(strID.c_str(), nullptr, false);
		const ImVec2 padding = ((flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));
		const float text_offset_x = g.FontSize + padding.x * 2;           // Collapser arrow width + Spacing
		const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);                    // Latch before ItemSize changes it
		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);  // Include collapser
		ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
		const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
		const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
		const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);

		bool previousState = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetID(strID.c_str()));

		if (is_mouse_x_over_arrow && isRowClicked)
			ImGui::SetNextItemOpen(!previousState);

		if (!isSelected && isAnyDescendantSelected(entity, isAnyDescendantSelected))
			ImGui::SetNextItemOpen(true);

		const bool opened = ImGui::TreeNodeWithIcon(nullptr, ImGui::GetID(strID.c_str()), flags, name, nullptr);

		int32_t rowIndex = ImGui::TableGetRowIndex();
		if (rowIndex >= m_FirstSelectedRow && rowIndex <= m_LastSelectedRow && !SelectionManager::IsSelected(entity.GetUUID()) && m_ShiftSelectionRunning)
		{
			SelectionManager::Select(s_ActiveSelectionContext, entity.GetUUID());

			if (SelectionManager::GetSelectionCount(s_ActiveSelectionContext) == (m_LastSelectedRow - m_FirstSelectedRow) + 1)
			{
				m_ShiftSelectionRunning = false;
			}
		}

		const std::string rightClickPopupID = fmt::format("{0}-ContextMenu", strID);

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem(rightClickPopupID.c_str()))
		{
			{
				UI::ScopedColour colText(ImGuiCol_Text, Colours::Theme::text);
				UI::ScopedColourStack entitySelection(ImGuiCol_Header, Colours::Theme::groupHeader,
					ImGuiCol_HeaderHovered, Colours::Theme::groupHeader,
					ImGuiCol_HeaderActive, Colours::Theme::groupHeader);

				if (!isSelected)
				{
					if (!Input::IsKeyDown(KeyCode::LeftControl))
						SelectionManager::DeselectAll();

					SelectionManager::Select(s_ActiveSelectionContext, entity.GetUUID());
				}

				if (isPrefab && isValidPrefab)
				{
					if (ImGui::MenuItem("Update Prefab"))
					{
						AssetHandle prefabAssetHandle = entity.GetComponent<PrefabComponent>().PrefabID;
						Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(prefabAssetHandle);
						if (prefab)
							prefab->Create(entity);
						else
							ANT_ERROR("Prefab has invalid asset handle: {0}", prefabAssetHandle);
					}
				}

				DrawEntityCreateMenu(entity);

				if (ImGui::MenuItem("Delete"))
					entityDeleted = true;

				ImGui::Separator();

				if (ImGui::MenuItem("Reset Transform to Mesh"))
					m_Context->ResetTransformsToMesh(entity, false);

				if (ImGui::MenuItem("Reset All Transforms to Mesh"))
					m_Context->ResetTransformsToMesh(entity, true);


				if (!m_EntityContextMenuPlugins.empty())
				{
					ImGui::Separator();

					if (ImGui::MenuItem("Set Transform to Editor Camera Transform"))
					{
						for (auto& func : m_EntityContextMenuPlugins)
						{
							func(entity);
						}
					}
				}
			}

			ImGui::EndPopup();
		}

		// Type column
		//------------
		if (isRowClicked)
		{
			if (wasRowRightClicked)
			{
				ImGui::OpenPopup(rightClickPopupID.c_str());
			}
			else
			{
				bool ctrlDown = Input::IsKeyDown(KeyCode::LeftControl) || Input::IsKeyDown(KeyCode::RightControl);
				bool shiftDown = Input::IsKeyDown(KeyCode::LeftShift) || Input::IsKeyDown(KeyCode::RightShift);
				if (shiftDown && SelectionManager::GetSelectionCount(s_ActiveSelectionContext) > 0)
				{
					SelectionManager::DeselectAll(s_ActiveSelectionContext);

					if (rowIndex < m_FirstSelectedRow)
					{
						m_LastSelectedRow = m_FirstSelectedRow;
						m_FirstSelectedRow = rowIndex;
					}
					else
					{
						m_LastSelectedRow = rowIndex;
					}

					m_ShiftSelectionRunning = true;
				}
				else if (!ctrlDown || shiftDown)
				{
					SelectionManager::DeselectAll();
					SelectionManager::Select(s_ActiveSelectionContext, entity.GetUUID());
					m_FirstSelectedRow = rowIndex;
					m_LastSelectedRow = -1;
				}
				else
				{
					if (isSelected)
						SelectionManager::Deselect(s_ActiveSelectionContext, entity.GetUUID());
					else
						SelectionManager::Select(s_ActiveSelectionContext, entity.GetUUID());
				}
			}

			ImGui::FocusWindow(ImGui::GetCurrentWindow());
		}

		if (missingMesh)
			ImGui::PopStyleColor();

		if (isSelected)
			ImGui::PopStyleColor();


		// Drag & Drop
		//------------
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			const auto& selectedEntities = SelectionManager::GetSelections(s_ActiveSelectionContext);
			UUID entityID = entity.GetUUID();

			if (!SelectionManager::IsSelected(s_ActiveSelectionContext, entityID))
			{
				const char* name = entity.Name().c_str();
				ImGui::TextUnformatted(name);
				ImGui::SetDragDropPayload("scene_entity_hierarchy", &entityID, 1 * sizeof(UUID));
			}
			else
			{
				for (const auto& selectedEntity : selectedEntities)
				{
					Entity e = m_Context->GetEntityWithUUID(selectedEntity);
					const char* name = e.Name().c_str();
					ImGui::TextUnformatted(name);
				}

				ImGui::SetDragDropPayload("scene_entity_hierarchy", selectedEntities.data(), selectedEntities.size() * sizeof(UUID));
			}

			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

			if (payload)
			{
				size_t count = payload->DataSize / sizeof(UUID);

				for (size_t i = 0; i < count; i++)
				{
					UUID droppedEntityID = *(((UUID*)payload->Data) + i);
					Entity droppedEntity = m_Context->GetEntityWithUUID(droppedEntityID);
					m_Context->ParentEntity(droppedEntity, entity);
				}
			}

			ImGui::EndDragDropTarget();
		}


		ImGui::TableNextColumn();
		if (isPrefab)
		{
			UI::ShiftCursorX(edgeOffset * 3.0f);

			if (isSelected)
				ImGui::PushStyleColor(ImGuiCol_Text, Colours::Theme::backgroundDark);

			ImGui::TextUnformatted("Prefab");
			ImGui::PopStyleColor();
		}

		// Draw children
		//--------------

		if (opened)
		{
			for (auto child : entity.Children())
				DrawEntityNode(m_Context->GetEntityWithUUID(child), "");

			ImGui::TreePop();
		}

		// Defer deletion until end of node UI
		if (entityDeleted)
		{
			// NOTE(Peter): Intentional copy since DestroyEntity would call EditorLayer::OnEntityDeleted which deselects the entity
			auto selectedEntities = SelectionManager::GetSelections(s_ActiveSelectionContext);
			for (auto entityID : selectedEntities)
				m_Context->DestroyEntity(m_Context->GetEntityWithUUID(entityID));
		}
	}


	enum class VectorAxis
	{
		X = BIT(0),
		Y = BIT(1),
		Z = BIT(2),
		W = BIT(3)
	};

	static bool DrawVec3Control(const std::string& label, glm::vec3& values, bool& manuallyEdited, float resetValue = 0.0f, float columnWidth = 100.0f, uint32_t renderMultiSelectAxes = 0)
	{
		bool modified = false;

		UI::PushID();
		ImGui::TableSetColumnIndex(0);
		UI::ShiftCursor(17.0f, 7.0f);

		ImGui::Text(label.c_str());
		UI::Draw::Underline(false, 0.0f, 2.0f);

		ImGui::TableSetColumnIndex(1);
		UI::ShiftCursor(7.0f, 0.0f);

		{
			const float spacingX = 8.0f;
			UI::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2{ spacingX, 0.0f });
			UI::ScopedStyle padding(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 2.0f });

			{
				// Begin XYZ area
				UI::ScopedColour padding(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
				UI::ScopedColour frame(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));

				ImGui::BeginChild(ImGui::GetID((label + "fr").c_str()),
					ImVec2(ImGui::GetContentRegionAvail().x - spacingX, ImGui::GetFrameHeightWithSpacing() + 8.0f),
					false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			}
			const float framePadding = 2.0f;
			const float outlineSpacing = 1.0f;
			const float lineHeight = GImGui->Font->FontSize + framePadding * 2.0f;
			const ImVec2 buttonSize = { lineHeight + 2.0f, lineHeight };
			const float inputItemWidth = (ImGui::GetContentRegionAvail().x - spacingX) / 3.0f - buttonSize.x;

			UI::ShiftCursor(0.0f, framePadding);

			const ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			auto drawControl = [&](const std::string& label, float& value, const ImVec4& colourN,
				const ImVec4& colourH,
				const ImVec4& colourP, bool renderMultiSelect)
			{
				{
					UI::ScopedStyle buttonFrame(ImGuiStyleVar_FramePadding, ImVec2(framePadding, 0.0f));
					UI::ScopedStyle buttonRounding(ImGuiStyleVar_FrameRounding, 1.0f);
					UI::ScopedColourStack buttonColours(ImGuiCol_Button, colourN,
						ImGuiCol_ButtonHovered, colourH,
						ImGuiCol_ButtonActive, colourP);

					UI::ScopedFont buttonFont(boldFont);

					UI::ShiftCursorY(2.0f);
					if (ImGui::Button(label.c_str(), buttonSize))
					{
						value = resetValue;
						modified = true;
					}
				}

				ImGui::SameLine(0.0f, outlineSpacing);
				ImGui::SetNextItemWidth(inputItemWidth);
				UI::ShiftCursorY(-2.0f);
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, renderMultiSelect);
				bool wasTempInputActive = ImGui::TempInputIsActive(ImGui::GetID(("##" + label).c_str()));
				modified |= UI::DragFloat(("##" + label).c_str(), &value, 0.1f, 0.0f, 0.0f, "%.2f", 0);

				// NOTE(Peter): Ugly hack to make tabbing behave the same as Enter (e.g marking it as manually modified)
				if (modified && Input::IsKeyDown(KeyCode::Tab))
					manuallyEdited = true;

				if (ImGui::TempInputIsActive(ImGui::GetID(("##" + label).c_str())))
					modified = false;

				ImGui::PopItemFlag();

				if (!UI::IsItemDisabled())
					UI::DrawItemActivityOutline(2.0f, true, Colours::Theme::accent);

				if (wasTempInputActive)
					manuallyEdited |= ImGui::IsItemDeactivatedAfterEdit();
			};

			drawControl("X", values.x, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f }, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, renderMultiSelectAxes & (uint32_t)VectorAxis::X);

			ImGui::SameLine(0.0f, outlineSpacing);
			drawControl("Y", values.y, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f }, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, renderMultiSelectAxes & (uint32_t)VectorAxis::Y);

			ImGui::SameLine(0.0f, outlineSpacing);
			drawControl("Z", values.z, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f }, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, renderMultiSelectAxes & (uint32_t)VectorAxis::Z);

			ImGui::EndChild();
		}
		UI::PopID();

		return modified || manuallyEdited;
	}

	template<typename TComponent>
	void DrawMaterialTable(SceneHierarchyPanel* _this, const std::vector<UUID>& entities, Ref<MaterialTable> meshMaterialTable, Ref<MaterialTable> localMaterialTable)
	{
		if (UI::BeginTreeNode("Materials"))
		{
			UI::BeginPropertyGrid();

			if (localMaterialTable->GetMaterialCount() != meshMaterialTable->GetMaterialCount())
				localMaterialTable->SetMaterialCount(meshMaterialTable->GetMaterialCount());

			for (uint32_t i = 0; i < (uint32_t)localMaterialTable->GetMaterialCount(); i++)
			{
				if (i == meshMaterialTable->GetMaterialCount())
					ImGui::Separator();

				bool hasLocalMaterial = localMaterialTable->HasMaterial(i);
				bool hasMeshMaterial = meshMaterialTable->HasMaterial(i);

				std::string label = fmt::format("[Material {0}]", i);

				// NOTE(Peter): Fix for weird ImGui ID bug...
				std::string id = fmt::format("{0}-{1}", label, i);
				ImGui::PushID(id.c_str());

				UI::PropertyAssetReferenceSettings settings;
				if (hasMeshMaterial && !hasLocalMaterial)
				{
					AssetHandle meshMaterialAssetHandle = meshMaterialTable->GetMaterial(i);
					Ref<MaterialAsset> meshMaterialAsset = AssetManager::GetAsset<MaterialAsset>(meshMaterialAssetHandle);
					std::string meshMaterialName = meshMaterialAsset->GetMaterial()->GetName();
					if (meshMaterialName.empty())
						meshMaterialName = "Unnamed Material";

					AssetHandle materialAssetHandle = meshMaterialAsset->Handle;

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, entities.size() > 1 && _this->IsInconsistentPrimitive<AssetHandle, TComponent>([i](const TComponent& component)
						{
							Ref<MaterialTable> materialTable = nullptr;
							if constexpr (std::is_same_v<TComponent, MeshComponent>)
								materialTable = AssetManager::GetAsset<Mesh>(component.Mesh)->GetMaterials();
							else
								materialTable = AssetManager::GetAsset<StaticMesh>(component.StaticMesh)->GetMaterials();

							if (!materialTable || i >= materialTable->GetMaterialCount())
								return (AssetHandle)0;

							return materialTable->GetMaterial(i);
						}));

					UI::PropertyAssetReferenceTarget<MaterialAsset>(label.c_str(), meshMaterialName.c_str(), materialAssetHandle, [_this, &entities, i, localMaterialTable](Ref<MaterialAsset> materialAsset) mutable
						{
							Ref<Scene> context = _this->GetSceneContext();

							for (auto entityID : entities)
							{
								Entity entity = context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<TComponent>();

								if (materialAsset == 0)
									component.MaterialTable->ClearMaterial(i);
								else
									component.MaterialTable->SetMaterial(i, materialAsset->Handle);
							}
						}, settings);

					ImGui::PopItemFlag();
				}
				else
				{
					// hasMeshMaterial is false, hasLocalMaterial could be true or false
					AssetHandle materialAssetHandle = 0;
					if (hasLocalMaterial)
					{
						materialAssetHandle = localMaterialTable->GetMaterial(i);
						settings.AdvanceToNextColumn = false;
						settings.WidthOffset = ImGui::GetStyle().ItemSpacing.x + 28.0f;
					}

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, entities.size() > 1 && _this->IsInconsistentPrimitive<AssetHandle, TComponent>([i, localMaterialTable](const TComponent& component)
						{
							Ref<MaterialTable> materialTable = component.MaterialTable;

							if (!materialTable || i >= materialTable->GetMaterialCount())
								return (AssetHandle)0;

							if (!materialTable->HasMaterial(i))
								return (AssetHandle)0;

							return materialTable->GetMaterial(i);
						}));

					UI::PropertyAssetReferenceTarget<MaterialAsset>(label.c_str(), nullptr, materialAssetHandle, [_this, &entities, i, localMaterialTable](Ref<MaterialAsset> materialAsset) mutable
						{
							Ref<Scene> context = _this->GetSceneContext();

							for (auto entityID : entities)
							{
								Entity entity = context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<TComponent>();

								if (materialAsset == 0)
									component.MaterialTable->ClearMaterial(i);
								else
									component.MaterialTable->SetMaterial(i, materialAsset->Handle);
							}
						}, settings);

					ImGui::PopItemFlag();
				}

				if (hasLocalMaterial)
				{
					ImGui::SameLine();
					float prevItemHeight = ImGui::GetItemRectSize().y;
					if (ImGui::Button(UI::GenerateLabelID("X"), { prevItemHeight, prevItemHeight }))
					{
						Ref<Scene> context = _this->GetSceneContext();

						for (auto entityID : entities)
						{
							Entity entity = context->GetEntityWithUUID(entityID);
							auto& component = entity.GetComponent<TComponent>();

							component.MaterialTable->ClearMaterial(i);
						}
					}
					ImGui::NextColumn();
				}

				ImGui::PopID();
			}

			UI::EndPropertyGrid();
			UI::EndTreeNode();
		}
	}

	template<typename TComponent, typename... TIncompatibleComponents>
	void DrawSimpleAddComponentButton(SceneHierarchyPanel* _this, const std::string& name, Ref<Texture2D> icon = nullptr)
	{
		bool canAddComponent = false;

		for (const auto& entityID : SelectionManager::GetSelections(SceneHierarchyPanel::GetActiveSelectionContext()))
		{
			Entity entity = _this->GetSceneContext()->GetEntityWithUUID(entityID);
			if (!entity.HasComponent<TComponent>())
			{
				canAddComponent = true;
				break;
			}
		}

		if (!canAddComponent)
			return;

		if (icon == nullptr)
			icon = EditorResources::AssetIcon;

		const float rowHeight = 25.0f;
		auto* window = ImGui::GetCurrentWindow();
		window->DC.CurrLineSize.y = rowHeight;
		ImGui::TableNextRow(0, rowHeight);
		ImGui::TableSetColumnIndex(0);

		window->DC.CurrLineTextBaseOffset = 3.0f;

		const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
		const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x - 20,
									rowAreaMin.y + rowHeight };

		ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
		bool isRowHovered, held;
		bool isRowClicked = ImGui::ButtonBehavior(ImRect(rowAreaMin, rowAreaMax), ImGui::GetID(name.c_str()), &isRowHovered, &held, ImGuiButtonFlags_AllowItemOverlap);
		ImGui::SetItemAllowOverlap();
		ImGui::PopClipRect();

		auto fillRowWithColour = [](const ImColor& colour)
		{
			for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, colour, column);
		};

		if (isRowHovered)
			fillRowWithColour(Colours::Theme::background);

		UI::ShiftCursor(1.5f, 1.5f);
		UI::Image(icon, { rowHeight - 3.0f, rowHeight - 3.0f });
		UI::ShiftCursor(-1.5f, -1.5f);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-1);
		ImGui::TextUnformatted(name.c_str());

		if (isRowClicked)
		{
			for (const auto& entityID : SelectionManager::GetSelections(SceneHierarchyPanel::GetActiveSelectionContext()))
			{
				Entity entity = _this->GetSceneContext()->GetEntityWithUUID(entityID);

				if (sizeof...(TIncompatibleComponents) > 0 && entity.HasComponent<TIncompatibleComponents...>())
					continue;

				if (!entity.HasComponent<TComponent>())
					entity.AddComponent<TComponent>();
			}

			ImGui::CloseCurrentPopup();
		}
	}

	template<typename TComponent, typename... TIncompatibleComponents, typename OnAddedFunction>
	void DrawAddComponentButton(SceneHierarchyPanel* _this, const std::string& name, OnAddedFunction onComponentAdded, Ref<Texture2D> icon = nullptr)
	{
		bool canAddComponent = false;

		for (const auto& entityID : SelectionManager::GetSelections(SceneHierarchyPanel::GetActiveSelectionContext()))
		{
			Entity entity = _this->GetSceneContext()->GetEntityWithUUID(entityID);
			if (!entity.HasComponent<TComponent>())
			{
				canAddComponent = true;
				break;
			}
		}

		if (!canAddComponent)
			return;

		if (icon == nullptr)
			icon = EditorResources::AssetIcon;

		const float rowHeight = 25.0f;
		auto* window = ImGui::GetCurrentWindow();
		window->DC.CurrLineSize.y = rowHeight;
		ImGui::TableNextRow(0, rowHeight);
		ImGui::TableSetColumnIndex(0);

		window->DC.CurrLineTextBaseOffset = 3.0f;

		const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
		const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x - 20,
									rowAreaMin.y + rowHeight };

		ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
		bool isRowHovered, held;
		bool isRowClicked = ImGui::ButtonBehavior(ImRect(rowAreaMin, rowAreaMax), ImGui::GetID(name.c_str()), &isRowHovered, &held, ImGuiButtonFlags_AllowItemOverlap);
		ImGui::SetItemAllowOverlap();
		ImGui::PopClipRect();

		auto fillRowWithColour = [](const ImColor& colour)
		{
			for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, colour, column);
		};

		if (isRowHovered)
			fillRowWithColour(Colours::Theme::background);

		UI::ShiftCursor(1.5f, 1.5f);
		UI::Image(icon, { rowHeight - 3.0f, rowHeight - 3.0f });
		UI::ShiftCursor(-1.5f, -1.5f);
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-1);
		ImGui::TextUnformatted(name.c_str());

		if (isRowClicked)
		{
			for (const auto& entityID : SelectionManager::GetSelections(SceneHierarchyPanel::GetActiveSelectionContext()))
			{
				Entity entity = _this->GetSceneContext()->GetEntityWithUUID(entityID);

				if (sizeof...(TIncompatibleComponents) > 0 && entity.HasComponent<TIncompatibleComponents...>())
					continue;

				if (!entity.HasComponent<TComponent>())
				{
					auto& component = entity.AddComponent<TComponent>();
					onComponentAdded(entity, component);
				}
			}

			ImGui::CloseCurrentPopup();
		}
	}

	void SceneHierarchyPanel::DrawComponents(const std::vector<UUID>& entityIDs)
	{
		if (entityIDs.size() == 0)
			return;

		ImGui::AlignTextToFramePadding();

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		UI::ShiftCursor(4.0f, 4.0f);

		bool isHoveringID = false;
		const bool isMultiSelect = entityIDs.size() > 1;

		Entity firstEntity = m_Context->GetEntityWithUUID(entityIDs[0]);

		// Draw Tag Field
		{
			const float iconOffset = 6.0f;
			UI::ShiftCursor(4.0f, iconOffset);
			UI::Image(EditorResources::PencilIcon, ImVec2((float)EditorResources::PencilIcon->GetWidth(), (float)EditorResources::PencilIcon->GetHeight()),
				ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
				ImColor(128, 128, 128, 255).Value);

			ImGui::SameLine(0.0f, 4.0f);
			UI::ShiftCursorY(-iconOffset);

			const bool inconsistentTag = IsInconsistentString<TagComponent>([](const TagComponent& tagComponent) { return tagComponent.Tag; });
			const std::string& tag = (isMultiSelect && inconsistentTag) ? "---" : firstEntity.Name();

			char buffer[256];
			memset(buffer, 0, 256);
			buffer[0] = 0; // Setting the first byte to 0 makes checking if string is empty easier later.
			memcpy(buffer, tag.c_str(), tag.length());
			ImGui::PushItemWidth(contentRegionAvailable.x * 0.5f);
			UI::ScopedStyle frameBorder(ImGuiStyleVar_FrameBorderSize, 0.0f);
			UI::ScopedColour frameColour(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
			UI::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[0]);

			if (Input::IsKeyDown(KeyCode::F2) && (m_IsHierarchyOrPropertiesFocused || UI::IsWindowFocused("Viewport")) && !ImGui::IsAnyItemActive())
				ImGui::SetKeyboardFocusHere();

			if (ImGui::InputText("##Tag", buffer, 256))
			{
				for (auto entityID : entityIDs)
				{
					Entity entity = m_Context->GetEntityWithUUID(entityID);
					if (buffer[0] == 0)
						memcpy(buffer, "Unnamed Entity", 16); // if the entity has no name, the name will be set to Unnamed Entity, this prevents invisible entities in SHP.

					entity.GetComponent<TagComponent>().Tag = buffer;
				}
			}

			UI::DrawItemActivityOutline(2.0f, false, Colours::Theme::accent);

			isHoveringID = ImGui::IsItemHovered();

			ImGui::PopItemWidth();
		}

		ImGui::SameLine();
		UI::ShiftCursorX(-5.0f);

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		ImVec2 addTextSize = ImGui::CalcTextSize(" ADD        ");
		addTextSize.x += GImGui->Style.FramePadding.x * 2.0f;

		{
			UI::ScopedColourStack addCompButtonColours(ImGuiCol_Button, IM_COL32(70, 70, 70, 200),
				ImGuiCol_ButtonHovered, IM_COL32(70, 70, 70, 255),
				ImGuiCol_ButtonActive, IM_COL32(70, 70, 70, 150));

			ImGui::SameLine(contentRegionAvailable.x - (addTextSize.x + GImGui->Style.FramePadding.x));
			if (ImGui::Button(" ADD       ", ImVec2(addTextSize.x + 4.0f, lineHeight + 2.0f)))
				ImGui::OpenPopup("AddComponentPanel");

			const float pad = 4.0f;
			const float iconWidth = ImGui::GetFrameHeight() - pad * 2.0f;
			const float iconHeight = iconWidth;
			ImVec2 iconPos = ImGui::GetItemRectMax();
			iconPos.x -= iconWidth + pad;
			iconPos.y -= iconHeight + pad;
			ImGui::SetCursorScreenPos(iconPos);
			UI::ShiftCursor(-5.0f, -1.0f);

			UI::Image(EditorResources::PlusIcon, ImVec2(iconWidth, iconHeight),
				ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
				ImColor(160, 160, 160, 255).Value);
		}

		float addComponentPanelStartY = ImGui::GetCursorScreenPos().y;

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		{
			UI::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[0]);
			UI::ScopedStyle itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(5, 10));
			UI::ScopedStyle windowRounding(ImGuiStyleVar_PopupRounding, 4.0f);
			UI::ScopedStyle cellPadding(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0));

			static float addComponentPanelWidth = 250.0f;
			ImVec2 windowPos = ImGui::GetWindowPos();
			const float maxHeight = ImGui::GetContentRegionMax().y - 60.0f;

			ImGui::SetNextWindowPos({ windowPos.x + addComponentPanelWidth / 1.3f, addComponentPanelStartY });
			ImGui::SetNextWindowSizeConstraints({ 50.0f, 50.0f }, { addComponentPanelWidth, maxHeight });
			if (ImGui::BeginPopup("AddComponentPanel", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking))
			{
				// Setup Table
				if (ImGui::BeginTable("##component_table", 2, ImGuiTableFlags_SizingStretchSame))
				{
					ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed, addComponentPanelWidth * 0.15f);
					ImGui::TableSetupColumn("ComponentNames", ImGuiTableColumnFlags_WidthFixed, addComponentPanelWidth * 0.85f);

					DrawSimpleAddComponentButton<CameraComponent>(this, "Camera", EditorResources::CameraIcon);
					DrawSimpleAddComponentButton<MeshComponent, StaticMeshComponent>(this, "Mesh", EditorResources::MeshIcon);
					DrawSimpleAddComponentButton<StaticMeshComponent, MeshComponent>(this, "Static Mesh", EditorResources::StaticMeshIcon);
					DrawSimpleAddComponentButton<DirectionalLightComponent>(this, "Directional Light", EditorResources::DirectionalLightIcon);
					DrawSimpleAddComponentButton<PointLightComponent>(this, "Point Light", EditorResources::PointLightIcon);
					DrawSimpleAddComponentButton<SpotLightComponent>(this, "Spot Light", EditorResources::SpotLightIcon);
					DrawSimpleAddComponentButton<SkyLightComponent>(this, "Sky Light", EditorResources::SkyLightIcon);
					DrawSimpleAddComponentButton<ScriptComponent>(this, "Script", EditorResources::ScriptIcon);
					DrawSimpleAddComponentButton<SpriteRendererComponent>(this, "Sprite Renderer", EditorResources::SpriteIcon);
					DrawSimpleAddComponentButton<AnimationComponent>(this, "Animation", EditorResources::AnimationIcon);
					DrawAddComponentButton<TextComponent>(this, "Text", [](Entity entity, TextComponent& tc)
						{
							tc.FontHandle = Font::GetDefaultFont()->Handle;
						}, EditorResources::TextIcon);
					DrawSimpleAddComponentButton<RigidBody2DComponent>(this, "Rigidbody 2D", EditorResources::RigidBody2DIcon);
					DrawSimpleAddComponentButton<BoxCollider2DComponent>(this, "Box Collider 2D", EditorResources::BoxCollider2DIcon);
					DrawSimpleAddComponentButton<CircleCollider2DComponent>(this, "Circle Collider 2D", EditorResources::CircleCollider2DIcon);
					DrawSimpleAddComponentButton<RigidBodyComponent>(this, "Rigidbody", EditorResources::RigidBodyIcon);
					DrawSimpleAddComponentButton<CharacterControllerComponent>(this, "Character Controller", EditorResources::CharacterControllerIcon);
					DrawSimpleAddComponentButton<BoxColliderComponent>(this, "Box Collider", EditorResources::BoxColliderIcon);
					DrawSimpleAddComponentButton<SphereColliderComponent>(this, "Sphere Collider", EditorResources::SphereColliderIcon);
					DrawSimpleAddComponentButton<CapsuleColliderComponent>(this, "Capsule Collider", EditorResources::CapsuleColliderIcon);
					DrawAddComponentButton<MeshColliderComponent>(this, "Mesh Collider", [&](Entity entity, MeshColliderComponent& colliderComponent)
						{
							PhysicsSystem::GetOrCreateColliderAsset(entity, colliderComponent);
						}, EditorResources::MeshColliderIcon);
					DrawSimpleAddComponentButton<FixedJointComponent>(this, "Fixed Joint", EditorResources::FixedJointIcon);
					DrawSimpleAddComponentButton<AudioComponent>(this, "Audio", EditorResources::AudioIcon);
					DrawAddComponentButton<AudioListenerComponent>(this, "Audio Listener", [&](Entity entity, AudioListenerComponent& alc)
						{
							auto view = m_Context->GetAllEntitiesWith<AudioListenerComponent>();
							alc.Active = view.size() == 1;
							MiniAudioEngine::Get().RegisterNewListener(alc);
						}, EditorResources::AudioListenerIcon);

					ImGui::EndTable();
				}

				ImGui::EndPopup();
			}
		}

		const auto& editorSettings = ApplicationSettings::Get();
		if (editorSettings.AdvancedMode)
		{
			DrawComponent<PrefabComponent>("Prefab", [&](PrefabComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
				{
					UI::BeginPropertyGrid();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<UUID, PrefabComponent>([](const PrefabComponent& other) { return other.PrefabID; }));
					if (UI::PropertyInput("Prefab ID", (uint64_t&)firstComponent.PrefabID))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<PrefabComponent>().PrefabID = firstComponent.PrefabID;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<UUID, PrefabComponent>([](const PrefabComponent& other) { return other.EntityID; }));
					if (UI::PropertyInput("Entity ID", (uint64_t&)firstComponent.EntityID))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<PrefabComponent>().EntityID = firstComponent.EntityID;
						}
					}

					ImGui::PopItemFlag();

					UI::EndPropertyGrid();
				});
		}

		DrawComponent<TransformComponent>("Transform", [&](TransformComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::ScopedStyle spacing(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
				UI::ScopedStyle padding(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));

				ImGui::BeginTable("transformComponent", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoClip);
				ImGui::TableSetupColumn("label_column", 0, 100.0f);
				ImGui::TableSetupColumn("value_column", ImGuiTableColumnFlags_IndentEnable | ImGuiTableColumnFlags_NoClip, ImGui::GetContentRegionAvail().x - 100.0f);

				bool translationManuallyEdited = false;
				bool rotationManuallyEdited = false;
				bool scaleManuallyEdited = false;

				if (isMultiEdit)
				{
					uint32_t translationAxes = GetInconsistentVectorAxis<glm::vec3, TransformComponent>([](const TransformComponent& other) { return other.Translation; });
					uint32_t rotationAxes = GetInconsistentVectorAxis<glm::vec3, TransformComponent>([](const TransformComponent& other) { return other.GetRotationEuler(); });
					uint32_t scaleAxes = GetInconsistentVectorAxis<glm::vec3, TransformComponent>([](const TransformComponent& other) { return other.Scale; });

					glm::vec3 translation = firstComponent.Translation;
					glm::vec3 rotation = glm::degrees(firstComponent.GetRotationEuler());
					glm::vec3 scale = firstComponent.Scale;

					glm::vec3 oldTranslation = translation;
					glm::vec3 oldRotation = rotation;
					glm::vec3 oldScale = scale;

					ImGui::TableNextRow();
					bool changed = DrawVec3Control("Translation", translation, translationManuallyEdited, 0.0f, 100.0f, translationAxes);

					ImGui::TableNextRow();
					changed |= DrawVec3Control("Rotation", rotation, rotationManuallyEdited, 0.0f, 100.0f, rotationAxes);

					ImGui::TableNextRow();
					changed |= DrawVec3Control("Scale", scale, scaleManuallyEdited, 1.0f, 100.0f, scaleAxes);

					if (changed)
					{
						if (translationManuallyEdited || rotationManuallyEdited || scaleManuallyEdited)
						{
							translationAxes = GetInconsistentVectorAxis(translation, oldTranslation);
							rotationAxes = GetInconsistentVectorAxis(rotation, oldRotation);
							scaleAxes = GetInconsistentVectorAxis(scale, oldScale);

							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<TransformComponent>();

								if ((translationAxes & (uint32_t)VectorAxis::X) != 0)
									component.Translation.x = translation.x;
								if ((translationAxes & (uint32_t)VectorAxis::Y) != 0)
									component.Translation.y = translation.y;
								if ((translationAxes & (uint32_t)VectorAxis::Z) != 0)
									component.Translation.z = translation.z;

								glm::vec3 componentRotation = glm::degrees(component.GetRotationEuler());
								if ((rotationAxes & (uint32_t)VectorAxis::X) != 0)
									componentRotation.x = glm::radians(rotation.x);
								if ((rotationAxes & (uint32_t)VectorAxis::Y) != 0)
									componentRotation.y = glm::radians(rotation.y);
								if ((rotationAxes & (uint32_t)VectorAxis::Z) != 0)
									componentRotation.z = glm::radians(rotation.z);
								component.SetRotationEuler(componentRotation);

								if ((scaleAxes & (uint32_t)VectorAxis::X) != 0)
									component.Scale.x = scale.x;
								if ((scaleAxes & (uint32_t)VectorAxis::Y) != 0)
									component.Scale.y = scale.y;
								if ((scaleAxes & (uint32_t)VectorAxis::Z) != 0)
									component.Scale.z = scale.z;
							}
						}
						else
						{
							glm::vec3 translationDiff = translation - oldTranslation;
							glm::vec3 rotationDiff = rotation - oldRotation;
							glm::vec3 scaleDiff = scale - oldScale;

							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<TransformComponent>();

								component.Translation += translationDiff;
								glm::vec3 componentRotation = component.GetRotationEuler();
								componentRotation += glm::radians(rotationDiff);
								component.SetRotationEuler(componentRotation);
								component.Scale += scaleDiff;
							}
						}
					}
				}
				else
				{
					Entity entity = m_Context->GetEntityWithUUID(entities[0]);
					auto& component = entity.GetComponent<TransformComponent>();

					ImGui::TableNextRow();
					DrawVec3Control("Translation", component.Translation, translationManuallyEdited);

					ImGui::TableNextRow();
					glm::vec3 rotation = glm::degrees(component.GetRotationEuler());
					if (DrawVec3Control("Rotation", rotation, rotationManuallyEdited))
						component.SetRotationEuler(glm::radians(rotation));

					ImGui::TableNextRow();
					DrawVec3Control("Scale", component.Scale, scaleManuallyEdited, 1.0f);
				}

				ImGui::EndTable();

				UI::ShiftCursorY(-8.0f);
				UI::Draw::Underline();

				UI::ShiftCursorY(18.0f);
			}, EditorResources::TransformIcon);

		DrawComponent<MeshComponent>("Mesh", [&](MeshComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				AssetHandle meshHandle = firstComponent.Mesh;
				Ref<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
				UI::BeginPropertyGrid();
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<AssetHandle, MeshComponent>([](const MeshComponent& other) { return other.Mesh; }));
				UI::PropertyAssetReferenceError error;
				if (UI::PropertyAssetReferenceWithConversion<Mesh, MeshSource>("Mesh", meshHandle,
					[=](Ref<MeshSource> meshAsset)
					{
						if (m_MeshAssetConvertCallback && !isMultiEdit)
							m_MeshAssetConvertCallback(m_Context->GetEntityWithUUID(entities[0]), meshAsset);
					}, &error))
				{
					mesh = AssetManager::GetAsset<Mesh>(meshHandle);
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						auto& mc = entity.GetComponent<MeshComponent>();
						mc.Mesh = meshHandle;
						mc.BoneEntityIds = m_Context->FindBoneEntityIds(entity, entity.GetParent(), mesh);
						if (mesh)
						{
							// Validate submesh index	
							mc.SubmeshIndex = glm::clamp<uint32_t>(mc.SubmeshIndex, 0, (uint32_t)mesh->GetMeshSource()->GetSubmeshes().size() - 1);
							// TODO(Yan): maybe prompt for this, this isn't always expected behaviour	
							if (entity.HasComponent<MeshColliderComponent>())
							{
								//CookingFactory::CookMesh(mcc.CollisionMesh);	
							}
						}
					}
				}
					ImGui::PopItemFlag();
					if (error == UI::PropertyAssetReferenceError::InvalidMetadata)
					{
						if (m_InvalidMetadataCallback && !isMultiEdit)
							m_InvalidMetadataCallback(m_Context->GetEntityWithUUID(entities[0]), UI::s_PropertyAssetReferenceAssetHandle);
					}
					if (mesh)
					{
						uint32_t submeshIndex = firstComponent.SubmeshIndex;
						ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<uint32_t, MeshComponent>([](const MeshComponent& other) { return other.SubmeshIndex; }));
						if (UI::Property("Submesh Index", submeshIndex, 0, (uint32_t)mesh->GetMeshSource()->GetSubmeshes().size() - 1))
						{
							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& mc = entity.GetComponent<MeshComponent>();
								mc.SubmeshIndex = glm::clamp<uint32_t>(submeshIndex, 0, (uint32_t)mesh->GetMeshSource()->GetSubmeshes().size() - 1);
							}
						}
						ImGui::PopItemFlag();
					}
					UI::EndPropertyGrid();
					if (mesh && mesh->IsValid())
						DrawMaterialTable<MeshComponent>(this, entities, mesh->GetMaterials(), firstComponent.MaterialTable);
			}, EditorResources::MeshIcon);

		DrawComponent<StaticMeshComponent>("Static Mesh", [&](StaticMeshComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				Ref<StaticMesh> mesh = AssetManager::GetAsset<StaticMesh>(firstComponent.StaticMesh);
				AssetHandle meshHandle = firstComponent.StaticMesh;

				UI::BeginPropertyGrid();
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<AssetHandle, StaticMeshComponent>([](const StaticMeshComponent& other) { return other.StaticMesh; }));

				UI::PropertyAssetReferenceError error;
				if (UI::PropertyAssetReferenceWithConversion<StaticMesh, MeshSource>("Static Mesh", meshHandle,
					[=](Ref<MeshSource> meshAsset)
					{
						if (m_MeshAssetConvertCallback && !isMultiEdit)
							m_MeshAssetConvertCallback(m_Context->GetEntityWithUUID(entities[0]), meshAsset);
					}, &error))
				{
					mesh = AssetManager::GetAsset<StaticMesh>(meshHandle);

					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						auto& mc = entity.GetComponent<StaticMeshComponent>();
						mc.StaticMesh = meshHandle;

						// TODO(Yan): maybe prompt for this, this isn't always expected behaviour
						/*if (entity.HasComponent<MeshColliderComponent>() && mesh)
						{
							auto& mcc = entity.GetComponent<MeshColliderComponent>();
							mcc.CollisionMesh = mc.StaticMesh;
							CookingFactory::CookMesh(mcc.CollisionMesh);
						}*/
					}
				}

					ImGui::PopItemFlag();

					if (error == UI::PropertyAssetReferenceError::InvalidMetadata)
					{
						if (m_InvalidMetadataCallback && !isMultiEdit)
							m_InvalidMetadataCallback(m_Context->GetEntityWithUUID(entities[0]), UI::s_PropertyAssetReferenceAssetHandle);
					}

					UI::EndPropertyGrid();

					if (mesh && mesh->IsValid())
						DrawMaterialTable<StaticMeshComponent>(this, entities, mesh->GetMaterials(), firstComponent.MaterialTable);
			}, EditorResources::StaticMeshIcon);

		DrawComponent<AnimationComponent>("Animation", [&](AnimationComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<AssetHandle, AnimationComponent>([](const AnimationComponent& other) { return other.AnimationController; }));

				UI::PropertyAssetReferenceError error;

				// draw animation controller reference in error color if the animation controller does not look relevant to this entity
				// (e.g. this entity has no bone entities that match the skeleton that is animated by the animation controller)
				UI::PropertyAssetReferenceSettings settings;
				if (firstComponent.BoneEntityIds.size() == 0)
					settings.ButtonLabelColor = ImGui::ColorConvertU32ToFloat4(Colours::Theme::textError);

				if (UI::PropertyAssetReference<AnimationController>("Animation Controller", firstComponent.AnimationController, &error, settings))
				{
					auto animationController = AssetManager::GetAsset<AnimationController>(firstComponent.AnimationController);
					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						auto& anim = entity.GetComponent<AnimationComponent>();
						anim.AnimationController = firstComponent.AnimationController;
						anim.BoneEntityIds = m_Context->FindBoneEntityIds(entity, entity, animationController);
						if (animationController)
						{
							auto skeletonAsset = animationController->GetSkeletonAsset();
							if (skeletonAsset && skeletonAsset->IsValid())
							{
								const auto& skeleton = animationController->GetSkeletonAsset()->GetSkeleton();
								anim.AnimationData->Resize(skeleton.GetNumBones());
								anim.AnimationData->SetLocalTransforms(skeleton.GetBoneTranslations(), skeleton.GetBoneRotations(), skeleton.GetBoneScales());
							}
						}
					}
				}

				ImGui::PopItemFlag();

				if (error == UI::PropertyAssetReferenceError::InvalidMetadata) // TODO: error only reports invalid assets.  What about other errors, like: rig not matching animation skeleton?
				{
					if (m_InvalidMetadataCallback && !isMultiEdit)
						m_InvalidMetadataCallback(m_Context->GetEntityWithUUID(entities[0]), UI::s_PropertyAssetReferenceAssetHandle);
				}
				if (AssetManager::IsAssetHandleValid(firstComponent.AnimationController))
				{
					auto animationController = AssetManager::GetAsset<AnimationController>(firstComponent.AnimationController);

					////////////////////////////
					// TEMPORARY:  Move to animation preview panel (if we ever have such a thing)
					//
					size_t stateIndex = firstComponent.AnimationData->m_StateIndex;
					{
						if (stateIndex < animationController->GetNumStates())
						{
							auto state = animationController->GetAnimationState(stateIndex);
							UI::ScopedColour color(ImGuiCol_Text, state->IsValid() ? ImGui::ColorConvertU32ToFloat4(Colours::Theme::text) : ImGui::ColorConvertU32ToFloat4(Colours::Theme::textError));
							if (UI::Property("State", stateIndex))
							{
								for (auto entityID : entities)
								{
									Entity entity = m_Context->GetEntityWithUUID(entityID);
									auto& anim = entity.GetComponent<AnimationComponent>();
									animationController->SetStateIndex(std::clamp<uint32_t>((uint32_t)stateIndex, 0, (uint32_t)animationController->GetNumStates() - 1), *anim.AnimationData);
								}
							}
						}
					}

					if (UI::Property("Play Animation", firstComponent.AnimationData->m_IsAnimationPlaying))
					{
						for (auto entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							auto& anim = entity.GetComponent<AnimationComponent>();
							anim.AnimationData->m_IsAnimationPlaying = firstComponent.AnimationData->m_IsAnimationPlaying;
						}
					}

					if (UI::Property("Playback Speed", firstComponent.AnimationData->m_PlaybackSpeed, 0.1f, 0.0f, 100.0f)) // upper limit here is kinda arbitrary, but probably better than no limit
					{
						for (auto entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							auto& anim = entity.GetComponent<AnimationComponent>();
							anim.AnimationData->m_PlaybackSpeed = firstComponent.AnimationData->m_PlaybackSpeed;
						}
					}

					{
						UI::ScopedDisable disable(firstComponent.AnimationData->m_IsAnimationPlaying);
						if (UI::PropertySlider("Animation Time", firstComponent.AnimationData->m_AnimationTime, 0.0f, 1.0f))
						{
							for (auto entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& anim = entity.GetComponent<AnimationComponent>();
								anim.AnimationData->m_AnimationTime = firstComponent.AnimationData->m_AnimationTime;
							}
						}

					}
					//
					/////////////////////////////

					if (UI::Property("Enable Root Motion", firstComponent.EnableRootMotion))
					{
						for (auto entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							auto& anim = entity.GetComponent<AnimationComponent>();
							anim.EnableRootMotion = firstComponent.EnableRootMotion;
						}
					}

					{
						UI::ScopedDisable disable(!firstComponent.EnableRootMotion);
						if (UI::PropertyEntityReference("Root Motion Target Entity", firstComponent.RootMotionTarget, m_Context))
						{
							for (auto entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& anim = entity.GetComponent<AnimationComponent>();
								anim.RootMotionTarget = firstComponent.RootMotionTarget;
							}
						}
					}
				}
				UI::EndPropertyGrid();
			}, EditorResources::AnimationIcon);

		DrawComponent<CameraComponent>("Camera", [&](CameraComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				// Projection Type
				const char* projTypeStrings[] = { "Perspective", "Orthographic" };
				int currentProj = (int)firstComponent.Camera.GetProjectionType();
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<int, CameraComponent>([](const CameraComponent& other) { return (int)other.Camera.GetProjectionType(); }));
				if (UI::PropertyDropdown("Projection", projTypeStrings, 2, &currentProj))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CameraComponent>().Camera.SetProjectionType((SceneCamera::ProjectionType)currentProj);
					}
				}
				ImGui::PopItemFlag();

				// Perspective parameters
				if (firstComponent.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float verticalFOV = firstComponent.Camera.GetDegPerspectiveVerticalFOV();
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& other) { return other.Camera.GetDegPerspectiveVerticalFOV(); }));
					if (UI::Property("Vertical FOV", verticalFOV))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<CameraComponent>().Camera.SetDegPerspectiveVerticalFOV(verticalFOV);
						}
					}
					ImGui::PopItemFlag();

					float nearClip = firstComponent.Camera.GetPerspectiveNearClip();
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& other) { return other.Camera.GetPerspectiveNearClip(); }));
					if (UI::Property("Near Clip", nearClip))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<CameraComponent>().Camera.SetPerspectiveNearClip(nearClip);
						}
					}
					ImGui::PopItemFlag();

					float farClip = firstComponent.Camera.GetPerspectiveFarClip();
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& other) { return other.Camera.GetPerspectiveFarClip(); }));
					if (UI::Property("Far Clip", farClip))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<CameraComponent>().Camera.SetPerspectiveFarClip(farClip);
						}
					}
					ImGui::PopItemFlag();
				}

				// Orthographic parameters
				else if (firstComponent.Camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = firstComponent.Camera.GetOrthographicSize();
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& other) { return other.Camera.GetOrthographicSize(); }));
					if (UI::Property("Size", orthoSize))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<CameraComponent>().Camera.SetOrthographicSize(orthoSize);
						}
					}
					ImGui::PopItemFlag();

					float nearClip = firstComponent.Camera.GetOrthographicNearClip();
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& other) { return other.Camera.GetOrthographicNearClip(); }));
					if (UI::Property("Near Clip", nearClip))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<CameraComponent>().Camera.SetOrthographicNearClip(nearClip);
						}
					}
					ImGui::PopItemFlag();

					float farClip = firstComponent.Camera.GetOrthographicFarClip();
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CameraComponent>([](const CameraComponent& other) { return other.Camera.GetOrthographicFarClip(); }));
					if (UI::Property("Far Clip", farClip))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<CameraComponent>().Camera.SetOrthographicFarClip(farClip);
						}
					}
					ImGui::PopItemFlag();
				}

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, CameraComponent>([](const CameraComponent& other) { return other.Primary; }));
				if (UI::Property("Main Camera", firstComponent.Primary))
				{
					// Does this even make sense???
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CameraComponent>().Primary = firstComponent.Primary;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::CameraIcon);

		//DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](SpriteRendererComponent& firstComponent, std::vector<Entity>& entities, const bool isMultiEdit)
		//{
		//}, s_GearIcon);

		DrawComponent<TextComponent>("Text", [&](TextComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				const bool inconsistentText = IsInconsistentString<TextComponent>([](const TextComponent& other) { return other.TextString; });

				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && inconsistentText);
				if (UI::PropertyMultiline("Text String", firstComponent.TextString))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						auto& textComponent = entity.GetComponent<TextComponent>();
						textComponent.TextString = firstComponent.TextString;
						textComponent.TextHash = std::hash<std::string>()(textComponent.TextString);
					}
				}
				ImGui::PopItemFlag();

				{
					UI::PropertyAssetReferenceSettings settings;
					bool customFont = firstComponent.FontHandle != Font::GetDefaultFont()->Handle;
					if (customFont)
					{
						settings.AdvanceToNextColumn = false;
						settings.WidthOffset = ImGui::GetStyle().ItemSpacing.x + 28.0f;
					}

					const bool inconsistentFont = IsInconsistentPrimitive<AssetHandle, TextComponent>([](const TextComponent& other) { return other.FontHandle; });
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && inconsistentFont);
					if (UI::PropertyAssetReference<Font>("Font", firstComponent.FontHandle, nullptr, settings))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<TextComponent>().FontHandle = firstComponent.FontHandle;
						}
					}
					ImGui::PopItemFlag();

					if (customFont)
					{
						ImGui::SameLine();
						float prevItemHeight = ImGui::GetItemRectSize().y;
						if (ImGui::Button("X", { prevItemHeight, prevItemHeight }))
						{
							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								entity.GetComponent<TextComponent>().FontHandle = Font::GetDefaultFont()->Handle;
							}
						}
						ImGui::NextColumn();
					}
				}

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec4, TextComponent>([](const TextComponent& other) { return other.Color; }));
				if (UI::PropertyColor("Color", firstComponent.Color))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<TextComponent>().Color = firstComponent.Color;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, TextComponent>([](const TextComponent& other) { return other.LineSpacing; }));
				if (UI::Property("Line Spacing", firstComponent.LineSpacing, 0.01f))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<TextComponent>().LineSpacing = firstComponent.LineSpacing;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, TextComponent>([](const TextComponent& other) { return other.Kerning; }));
				if (UI::Property("Kerning", firstComponent.Kerning, 0.01f))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<TextComponent>().Kerning = firstComponent.Kerning;
					}
				}
				ImGui::PopItemFlag();

				UI::Separator();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, TextComponent>([](const TextComponent& other) { return other.MaxWidth; }));
				if (UI::Property("Max Width", firstComponent.MaxWidth))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<TextComponent>().MaxWidth = firstComponent.MaxWidth;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();

			}, EditorResources::TextIcon);

		DrawComponent<DirectionalLightComponent>("Directional Light", [&](DirectionalLightComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec3, DirectionalLightComponent>([](const DirectionalLightComponent& other) { return other.Radiance; }));
				if (UI::PropertyColor("Radiance", firstComponent.Radiance))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<DirectionalLightComponent>().Radiance = firstComponent.Radiance;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, DirectionalLightComponent>([](const DirectionalLightComponent& other) { return other.Intensity; }));
				if (UI::Property("Intensity", firstComponent.Intensity, 0.1f, 0.0f, 1000.0f))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<DirectionalLightComponent>().Intensity = firstComponent.Intensity;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, DirectionalLightComponent>([](const DirectionalLightComponent& other) { return other.CastShadows; }));
				if (UI::Property("Cast Shadows", firstComponent.CastShadows))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<DirectionalLightComponent>().CastShadows = firstComponent.CastShadows;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, DirectionalLightComponent>([](const DirectionalLightComponent& other) { return other.SoftShadows; }));
				if (UI::Property("Soft Shadows", firstComponent.SoftShadows))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<DirectionalLightComponent>().SoftShadows = firstComponent.SoftShadows;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, DirectionalLightComponent>([](const DirectionalLightComponent& other) { return other.LightSize; }));
				if (UI::Property("Source Size", firstComponent.LightSize))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<DirectionalLightComponent>().LightSize = firstComponent.LightSize;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::DirectionalLightIcon);

		DrawComponent<PointLightComponent>("Point Light", [&](PointLightComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec3, PointLightComponent>([](const PointLightComponent& other) { return other.Radiance; }));
				if (UI::PropertyColor("Radiance", firstComponent.Radiance))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<PointLightComponent>().Radiance = firstComponent.Radiance;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, PointLightComponent>([](const PointLightComponent& other) { return other.Intensity; }));
				if (UI::Property("Intensity", firstComponent.Intensity, 0.05f, 0.0f, 500.0f))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<PointLightComponent>().Intensity = firstComponent.Intensity;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, PointLightComponent>([](const PointLightComponent& other) { return other.Radius; }));
				if (UI::Property("Radius", firstComponent.Radius, 0.1f, 0.0f, std::numeric_limits<float>::max()))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<PointLightComponent>().Radius = firstComponent.Radius;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, PointLightComponent>([](const PointLightComponent& other) { return other.Falloff; }));
				if (UI::Property("Falloff", firstComponent.Falloff, 0.005f, 0.0f, 1.0f))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<PointLightComponent>().Falloff = firstComponent.Falloff;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::PointLightIcon);

		DrawComponent<SpotLightComponent>("Spot Light", [&](SpotLightComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec3, SpotLightComponent>([](const SpotLightComponent& other) { return other.Radiance; }));
				if (UI::PropertyColor("Radiance", firstComponent.Radiance))
				{
					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpotLightComponent>().Radiance = firstComponent.Radiance;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SpotLightComponent>([](const SpotLightComponent& other) { return other.Intensity; }));
				if (UI::Property("Intensity", firstComponent.Intensity, 0.1f, 0.0f, 1000.0f))
				{
					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpotLightComponent>().Intensity = glm::clamp(firstComponent.Intensity, 0.0f, 1000.0f);
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SpotLightComponent>([](const SpotLightComponent& other) { return other.Angle; }));
				if (UI::Property("Angle", firstComponent.Angle))
				{
					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpotLightComponent>().Angle = glm::clamp(firstComponent.Angle, 0.1f, 180.0f);
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SpotLightComponent>([](const SpotLightComponent& other) { return other.AngleAttenuation; }));
				if (UI::Property("Angle Attenuation", firstComponent.AngleAttenuation, 0.01f, 0.0f, std::numeric_limits<float>::max()))
				{
					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpotLightComponent>().AngleAttenuation = glm::max(firstComponent.AngleAttenuation, 0.0f);
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SpotLightComponent>([](const SpotLightComponent& other) { return other.Falloff; }));
				if (UI::Property("Falloff", firstComponent.Falloff, 0.01f, 0.0f, 1.0f))
				{
					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpotLightComponent>().Falloff = glm::clamp(firstComponent.Falloff, 0.0f, 1.0f);
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SpotLightComponent>([](const SpotLightComponent& other) { return other.Range; }));
				if (UI::Property("Range", firstComponent.Range, 0.1f, 0.0f, std::numeric_limits<float>::max()))
				{
					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpotLightComponent>().Range = glm::max(firstComponent.Range, 0.0f);
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, SpotLightComponent>([](const SpotLightComponent& other) { return other.CastsShadows; }));
				if (UI::Property("Cast Shadows", firstComponent.CastsShadows))
				{
					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpotLightComponent>().CastsShadows = firstComponent.CastsShadows;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, SpotLightComponent>([](const SpotLightComponent& other) { return other.SoftShadows; }));
				if (UI::Property("Soft Shadows", firstComponent.SoftShadows))
				{
					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpotLightComponent>().SoftShadows = firstComponent.SoftShadows;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::SpotLightIcon);

		DrawComponent<SkyLightComponent>("Sky Light", [&](SkyLightComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<AssetHandle, SkyLightComponent>([](const SkyLightComponent& other) { return other.SceneEnvironment; }));
				if (UI::PropertyAssetReference<Environment>("Environment Map", firstComponent.SceneEnvironment))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SkyLightComponent>().SceneEnvironment = firstComponent.SceneEnvironment;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SkyLightComponent>([](const SkyLightComponent& other) { return other.Intensity; }));
				if (UI::Property("Intensity", firstComponent.Intensity, 0.01f, 0.0f, 5.0f))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SkyLightComponent>().Intensity = firstComponent.Intensity;
					}
				}
				ImGui::PopItemFlag();

				if (AssetManager::IsAssetHandleValid(firstComponent.SceneEnvironment))
				{
					auto environment = AssetManager::GetAsset<Environment>(firstComponent.SceneEnvironment);
					bool lodChanged = false;
					if (environment && environment->RadianceMap)
					{
						ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<uint32_t, SkyLightComponent>([](const SkyLightComponent& other)
							{
								auto otherEnv = AssetManager::GetAsset<Environment>(other.SceneEnvironment);
								return otherEnv->RadianceMap->GetMipLevelCount();
							}));

						lodChanged = UI::PropertySlider("Lod", firstComponent.Lod, 0.0f, static_cast<float>(environment->RadianceMap->GetMipLevelCount()));
						ImGui::PopItemFlag();
					}
					else
					{
						UI::BeginDisabled();
						UI::PropertySlider("Lod", firstComponent.Lod, 0.0f, 10.0f);
						UI::EndDisabled();
					}
				}

				ImGui::Separator();

				const bool isInconsistentDynamicSky = IsInconsistentPrimitive<bool, SkyLightComponent>([](const SkyLightComponent& other) { return other.DynamicSky; });
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && isInconsistentDynamicSky);
				if (UI::Property("Dynamic Sky", firstComponent.DynamicSky))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SkyLightComponent>().DynamicSky = firstComponent.DynamicSky;
					}
				}
				ImGui::PopItemFlag();

				if (!isInconsistentDynamicSky || !isMultiEdit)
				{
					bool changed = false;

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SkyLightComponent>([](const SkyLightComponent& other) { return other.TurbidityAzimuthInclination.x; }));
					if (UI::Property("Turbidity", firstComponent.TurbidityAzimuthInclination.x, 0.01f, 1.8f, std::numeric_limits<float>::max()))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<SkyLightComponent>().TurbidityAzimuthInclination.x = firstComponent.TurbidityAzimuthInclination.x;
						}

						changed = true;
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SkyLightComponent>([](const SkyLightComponent& other) { return other.TurbidityAzimuthInclination.y; }));
					if (UI::Property("Azimuth", firstComponent.TurbidityAzimuthInclination.y, 0.01f))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<SkyLightComponent>().TurbidityAzimuthInclination.y = firstComponent.TurbidityAzimuthInclination.y;
						}

						changed = true;
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SkyLightComponent>([](const SkyLightComponent& other) { return other.TurbidityAzimuthInclination.z; }));
					if (UI::Property("Inclination", firstComponent.TurbidityAzimuthInclination.z, 0.01f))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<SkyLightComponent>().TurbidityAzimuthInclination.z = firstComponent.TurbidityAzimuthInclination.z;
						}

						changed = true;
					}
					ImGui::PopItemFlag();

					if (changed)
					{
						if (AssetManager::IsMemoryAsset(firstComponent.SceneEnvironment))
						{
							Ref<TextureCube> preethamEnv = Renderer::CreatePreethamSky(firstComponent.TurbidityAzimuthInclination.x, firstComponent.TurbidityAzimuthInclination.y, firstComponent.TurbidityAzimuthInclination.z);
							Ref<Environment> env = AssetManager::GetAsset<Environment>(firstComponent.SceneEnvironment);
							if (env)
							{
								env->RadianceMap = preethamEnv;
								env->IrradianceMap = preethamEnv;
							}
						}
						else
						{
							Ref<TextureCube> preethamEnv = Renderer::CreatePreethamSky(firstComponent.TurbidityAzimuthInclination.x, firstComponent.TurbidityAzimuthInclination.y, firstComponent.TurbidityAzimuthInclination.z);
							firstComponent.SceneEnvironment = AssetManager::CreateMemoryOnlyAsset<Environment>(preethamEnv, preethamEnv);
						}

						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<SkyLightComponent>().SceneEnvironment = firstComponent.SceneEnvironment;
						}
					}
				}
				UI::EndPropertyGrid();
			}, EditorResources::SkyLightIcon);

		DrawComponent<ScriptComponent>("Script", [=](ScriptComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit) mutable
			{
				UI::BeginPropertyGrid();

				const bool inconsistentScriptClass = IsInconsistentPrimitive<AssetHandle, ScriptComponent>([](const ScriptComponent& other) { return other.ScriptClassHandle; });
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && inconsistentScriptClass);

				bool isError = !ScriptEngine::IsModuleValid(firstComponent.ScriptClassHandle);
				const UI::PropertyAssetReferenceSettings c_AssetRefSettings = { true, false, 0.0f, true, (isError && !inconsistentScriptClass) ? ImVec4(0.9f, 0.2f, 0.2f, 1.0f) : ImGui::ColorConvertU32ToFloat4(Colours::Theme::text), ImGui::ColorConvertU32ToFloat4(Colours::Theme::textError), true };

				AssetHandle oldAssetHandle = firstComponent.ScriptClassHandle;

				if (UI::PropertyAssetReference<ScriptAsset>("Script Class", firstComponent.ScriptClassHandle, nullptr, c_AssetRefSettings))
				{
					isError = !ScriptEngine::IsModuleValid(firstComponent.ScriptClassHandle);

					for (auto entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						auto& sc = entity.GetComponent<ScriptComponent>();
						sc.ScriptClassHandle = firstComponent.ScriptClassHandle;

						if (isError)
						{
							bool wasCleared = sc.ScriptClassHandle == 0;
							if (wasCleared)
								sc.ScriptClassHandle = oldAssetHandle; // NOTE(Peter): We need the old asset handle to properly shutdown an entity (at least during runtime)

							ScriptEngine::ShutdownScriptEntity(entity);

							if (wasCleared)
								sc.ScriptClassHandle = 0;
						}
						else
						{
							ScriptEngine::InitializeScriptEntity(entity);
						}
					}
				}

				ImGui::PopItemFlag();

				UI::EndPropertyGrid();

				// NOTE(Peter): Editing fields doesn't really work if there's inconsistencies with the script classes...
				if (!isError && !inconsistentScriptClass && firstComponent.FieldIDs.size() > 0)
				{
					UI::BeginPropertyGrid();
					Entity firstEntity = m_Context->GetEntityWithUUID(entities[0]);

					for (auto fieldID : firstComponent.FieldIDs)
					{
						FieldInfo* field = ScriptCache::GetFieldByID(fieldID);
						Ref<FieldStorageBase> storage = ScriptEngine::GetFieldStorage(firstEntity, field->ID);

						/*const bool isHidden = field->HasAttribute("Ant.HideFromEditorAttribute") || !field->IsWritable() || !storage->IsLive();

						if (isHidden && !ApplicationSettings::Get().ShowHiddenFields)
							continue;

						UI::ScopedDisable disable(isHidden);*/
						std::string fieldName = field->DisplayName.empty() ? Utils::String::SubStr(field->Name, field->Name.find(':') + 1) : field->DisplayName;

						// TODO(Peter): Update field input to display "---" when there's mixed values
						if (field->IsArray())
						{
							if (UI::DrawFieldArray(m_Context, fieldName, storage.As<ArrayFieldStorage>()))
							{
								for (auto entityID : entities)
								{
									/*Entity entity = m_Context->GetEntityWithUUID(entityID);
									const auto& sc = entity.GetComponent<ScriptComponent>();
									storage->CopyData(firstComponent.ManagedInstance, sc.ManagedInstance);*/
								}
							}
						}
						else
						{
							if (UI::DrawFieldValue(m_Context, fieldName, storage.As<FieldStorage>()))
							{
								for (auto entityID : entities)
								{
									/*Entity entity = m_Context->GetEntityWithUUID(entityID);
									const auto& sc = entity.GetComponent<ScriptComponent>();
									storage->CopyData(firstComponent.ManagedInstance, sc.ManagedInstance);*/
								}
							}
						}
					}

					UI::EndPropertyGrid();
				}

			}, EditorResources::ScriptIcon);

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", [&](SpriteRendererComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec4, SpriteRendererComponent>([](const SpriteRendererComponent& other) { return other.Color; }));
				if (UI::PropertyColor("Color", firstComponent.Color))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpriteRendererComponent>().Color = firstComponent.Color;
					}
				}
				ImGui::PopItemFlag();

				{
					UI::PropertyAssetReferenceSettings settings;
					bool textureSet = firstComponent.Texture != 0;
					if (textureSet)
					{
						settings.AdvanceToNextColumn = false;
						settings.WidthOffset = ImGui::GetStyle().ItemSpacing.x + 28.0f;
					}

					const bool inconsistentTexture = IsInconsistentPrimitive<AssetHandle, SpriteRendererComponent>([](const SpriteRendererComponent& other) { return other.Texture; });
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && inconsistentTexture);
					if (UI::PropertyAssetReference<Texture2D>("Texture", firstComponent.Texture, nullptr, settings))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<SpriteRendererComponent>().Texture = firstComponent.Texture;
						}
					}
					ImGui::PopItemFlag();

					if (textureSet)
					{
						ImGui::SameLine();
						float prevItemHeight = ImGui::GetItemRectSize().y;
						if (ImGui::Button("X", { prevItemHeight, prevItemHeight }))
						{
							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								entity.GetComponent<SpriteRendererComponent>().Texture = 0;
							}
						}
						ImGui::NextColumn();
					}
				}

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SpriteRendererComponent>([](const SpriteRendererComponent& other) { return other.TilingFactor; }));
				if (UI::Property("Tiling Factor", firstComponent.TilingFactor))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpriteRendererComponent>().TilingFactor = firstComponent.TilingFactor;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec2, SpriteRendererComponent>([](const SpriteRendererComponent& other) { return other.UVStart; }));
				if (UI::Property("UV Start", firstComponent.UVStart))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpriteRendererComponent>().UVStart = firstComponent.UVStart;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec2, SpriteRendererComponent>([](const SpriteRendererComponent& other) { return other.UVEnd; }));
				if (UI::Property("UV End", firstComponent.UVEnd))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SpriteRendererComponent>().UVEnd = firstComponent.UVEnd;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::SpriteIcon);

		DrawComponent<RigidBody2DComponent>("Rigidbody 2D", [&](RigidBody2DComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				// Rigidbody2D Type
				const char* rb2dTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<int, RigidBody2DComponent>([](const RigidBody2DComponent& other) { return (int)other.BodyType; }));
				if (UI::PropertyDropdown("Type", rb2dTypeStrings, 3, (int*)&firstComponent.BodyType))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<RigidBody2DComponent>().BodyType = firstComponent.BodyType;
					}
				}
				ImGui::PopItemFlag();

				if (firstComponent.BodyType == RigidBody2DComponent::Type::Dynamic)
				{
					UI::BeginPropertyGrid();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, RigidBody2DComponent>([](const RigidBody2DComponent& other) { return other.FixedRotation; }));
					if (UI::Property("Fixed Rotation", firstComponent.FixedRotation))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBody2DComponent>().FixedRotation = firstComponent.FixedRotation;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, RigidBody2DComponent>([](const RigidBody2DComponent& other) { return other.Mass; }));
					if (UI::Property("Mass", firstComponent.Mass))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBody2DComponent>().Mass = firstComponent.Mass;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, RigidBody2DComponent>([](const RigidBody2DComponent& other) { return other.LinearDrag; }));
					if (UI::Property("Linear Drag", firstComponent.LinearDrag))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBody2DComponent>().LinearDrag = firstComponent.LinearDrag;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, RigidBody2DComponent>([](const RigidBody2DComponent& other) { return other.AngularDrag; }));
					if (UI::Property("Angular Drag", firstComponent.AngularDrag))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBody2DComponent>().AngularDrag = firstComponent.AngularDrag;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, RigidBody2DComponent>([](const RigidBody2DComponent& other) { return other.GravityScale; }));
					if (UI::Property("Gravity Scale", firstComponent.GravityScale))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBody2DComponent>().GravityScale = firstComponent.GravityScale;
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, RigidBody2DComponent>([](const RigidBody2DComponent& other) { return other.IsBullet; }));
					if (UI::Property("Is Bullet", firstComponent.IsBullet))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBody2DComponent>().IsBullet = firstComponent.IsBullet;
						}
					}
					ImGui::PopItemFlag();

					UI::EndPropertyGrid();
				}

				UI::EndPropertyGrid();
			}, EditorResources::RigidBody2DIcon);

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", [&](BoxCollider2DComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec2, BoxCollider2DComponent>([](const BoxCollider2DComponent& other) { return other.Offset; }));
				if (UI::Property("Offset", firstComponent.Offset))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<BoxCollider2DComponent>().Offset = firstComponent.Offset;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec2, BoxCollider2DComponent>([](const BoxCollider2DComponent& other) { return other.Size; }));
				if (UI::Property("Size", firstComponent.Size))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<BoxCollider2DComponent>().Size = firstComponent.Size;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, BoxCollider2DComponent>([](const BoxCollider2DComponent& other) { return other.Density; }));
				if (UI::Property("Density", firstComponent.Density))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<BoxCollider2DComponent>().Density = firstComponent.Density;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, BoxCollider2DComponent>([](const BoxCollider2DComponent& other) { return other.Friction; }));
				if (UI::Property("Friction", firstComponent.Friction))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<BoxCollider2DComponent>().Friction = firstComponent.Friction;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::BoxCollider2DIcon);

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", [&](CircleCollider2DComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec2, CircleCollider2DComponent>([](const CircleCollider2DComponent& other) { return other.Offset; }));
				if (UI::Property("Offset", firstComponent.Offset))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CircleCollider2DComponent>().Offset = firstComponent.Offset;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CircleCollider2DComponent>([](const CircleCollider2DComponent& other) { return other.Radius; }));
				if (UI::Property("Radius", firstComponent.Radius))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CircleCollider2DComponent>().Radius = firstComponent.Radius;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CircleCollider2DComponent>([](const CircleCollider2DComponent& other) { return other.Density; }));
				if (UI::Property("Density", firstComponent.Density))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CircleCollider2DComponent>().Density = firstComponent.Density;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CircleCollider2DComponent>([](const CircleCollider2DComponent& other) { return other.Friction; }));
				if (UI::Property("Friction", firstComponent.Friction))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CircleCollider2DComponent>().Friction = firstComponent.Friction;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::CircleCollider2DIcon);

		DrawComponent<RigidBodyComponent>("Rigidbody", [&](RigidBodyComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				static const char* s_RigidBodyTypeNames[] = { "Static", "Dynamic" };

				if (!PhysicsLayerManager::IsLayerValid(firstComponent.LayerID))
				{
					for (auto& entityID : entityIDs)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<RigidBodyComponent>().LayerID = 0;

						if (m_Context->IsPlaying())
						{
							auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
							actor->SetSimulationData(firstComponent.LayerID);
						}
					}
				}

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<RigidBodyComponent::Type, RigidBodyComponent>([](const RigidBodyComponent& other) { return other.BodyType; }));
				if (UI::PropertyDropdown<RigidBodyComponent::Type, uint8_t>("Type", s_RigidBodyTypeNames, 2, firstComponent.BodyType))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<RigidBodyComponent>().BodyType = firstComponent.BodyType;

						if (m_Context->IsPlaying())
						{
							m_Context->GetPhysicsScene()->RemoveActor(entity);
							m_Context->GetPhysicsScene()->CreateActor(entity);
						}
					}
				}
				ImGui::PopItemFlag();

				const auto& layerNames = PhysicsLayerManager::GetLayerNames();
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, entityIDs.size() > 1 && IsInconsistentPrimitive<int, RigidBodyComponent>([](const RigidBodyComponent& other) { return other.LayerID; }));
				ImGui::PushItemWidth(125.0f);
				uint32_t oldLayer = firstComponent.LayerID;
				if (UI::PropertyDropdown("Layer", layerNames, (int32_t)layerNames.size(), (int*)&firstComponent.LayerID))
				{
					for (auto& entityID : entityIDs)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);

						if (m_Context->IsPlaying())
						{
							auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
							firstComponent.LayerID = actor->SetSimulationData(firstComponent.LayerID) ? firstComponent.LayerID : oldLayer;
						}

						entity.GetComponent<RigidBodyComponent>().LayerID = firstComponent.LayerID;
					}
				}
				ImGui::PopItemWidth();
				ImGui::PopItemFlag();

				if (firstComponent.BodyType == RigidBodyComponent::Type::Dynamic)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, RigidBodyComponent>([](const RigidBodyComponent& other) { return other.IsKinematic; }));
					if (UI::Property("Is Kinematic", firstComponent.IsKinematic))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBodyComponent>().IsKinematic = firstComponent.IsKinematic;

							if (m_Context->IsPlaying())
							{
								auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
								actor->SetKinematic(firstComponent.IsKinematic);
							}
						}
					}
					ImGui::PopItemFlag();

					static const char* s_CollisionDetectionNames[] = { "Discrete", "Continuous", "Continuous Speculative" };
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<int, RigidBodyComponent>([](const RigidBodyComponent& other) { return (int)other.CollisionDetection; }));
					if (UI::PropertyDropdown("Collision Detection", s_CollisionDetectionNames, 3, firstComponent.CollisionDetection))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBodyComponent>().CollisionDetection = firstComponent.CollisionDetection;
						}
					}
					ImGui::PopItemFlag();

					UI::BeginPropertyGrid();
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, RigidBodyComponent>([](const RigidBodyComponent& other) { return other.Mass; }));
					if (UI::Property("Mass", firstComponent.Mass))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBodyComponent>().Mass = firstComponent.Mass;

							if (m_Context->IsPlaying())
							{
								auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
								actor->SetMass(firstComponent.Mass);
							}
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, RigidBodyComponent>([](const RigidBodyComponent& other) { return other.LinearDrag; }));
					if (UI::Property("Linear Drag", firstComponent.LinearDrag))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBodyComponent>().LinearDrag = firstComponent.LinearDrag;

							if (m_Context->IsPlaying())
							{
								auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
								actor->SetLinearDrag(firstComponent.LinearDrag);
							}
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, RigidBodyComponent>([](const RigidBodyComponent& other) { return other.AngularDrag; }));
					if (UI::Property("Angular Drag", firstComponent.AngularDrag))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBodyComponent>().AngularDrag = firstComponent.AngularDrag;

							if (m_Context->IsPlaying())
							{
								auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
								actor->SetAngularDrag(firstComponent.AngularDrag);
							}
						}
					}
					ImGui::PopItemFlag();

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, RigidBodyComponent>([](const RigidBodyComponent& other) { return other.DisableGravity; }));
					if (UI::Property("Disable Gravity", firstComponent.DisableGravity))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<RigidBodyComponent>().DisableGravity = firstComponent.DisableGravity;

							if (m_Context->IsPlaying())
							{
								auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
								actor->SetGravityEnabled(!firstComponent.DisableGravity);
							}
						}
					}
					ImGui::PopItemFlag();
					UI::EndPropertyGrid();

					if (UI::BeginTreeNode("Constraints", false))
					{
						UI::BeginPropertyGrid();

						bool translationX = (firstComponent.LockFlags & (uint8_t)ActorLockFlag::TranslationX);
						bool translationY = (firstComponent.LockFlags & (uint8_t)ActorLockFlag::TranslationY);
						bool translationZ = (firstComponent.LockFlags & (uint8_t)ActorLockFlag::TranslationZ);
						bool rotationX = (firstComponent.LockFlags & (uint8_t)ActorLockFlag::RotationX);
						bool rotationY = (firstComponent.LockFlags & (uint8_t)ActorLockFlag::RotationY);
						bool rotationZ = (firstComponent.LockFlags & (uint8_t)ActorLockFlag::RotationZ);

						UI::BeginCheckboxGroup("Freeze Position");
						//UI::PushMultipleValuesFlag<bool, RigidBodyComponent>([](const RigidBodyComponent& other) { return (other.LockFlags & (uint8_t)ActorLockFlag::TranslationX); });
						ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<uint8_t, RigidBodyComponent>([](const RigidBodyComponent& other)
							{
								return (other.LockFlags & (uint8_t)ActorLockFlag::TranslationX);
							}));

						if (UI::PropertyCheckboxGroup("X", translationX))
						{
							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<RigidBodyComponent>();

								if (translationX)
									component.LockFlags |= (uint8_t)ActorLockFlag::TranslationX;
								else
									component.LockFlags &= ~(uint8_t)ActorLockFlag::TranslationX;

								if (m_Context->IsPlaying())
								{
									auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
									actor->SetLockFlag(ActorLockFlag::TranslationX, translationX, true);
								}
							}
						}
						ImGui::PopItemFlag();

						ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<uint8_t, RigidBodyComponent>([](const RigidBodyComponent& other)
							{
								return (other.LockFlags & (uint8_t)ActorLockFlag::TranslationY);
							}));
						if (UI::PropertyCheckboxGroup("Y", translationY))
						{
							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<RigidBodyComponent>();

								if (translationY)
									component.LockFlags |= (uint8_t)ActorLockFlag::TranslationY;
								else
									component.LockFlags &= ~(uint8_t)ActorLockFlag::TranslationY;

								if (m_Context->IsPlaying())
								{
									auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
									actor->SetLockFlag(ActorLockFlag::TranslationY, translationY, true);
								}
							}
						}
						ImGui::PopItemFlag();

						ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<uint8_t, RigidBodyComponent>([](const RigidBodyComponent& other)
							{
								return (other.LockFlags & (uint8_t)ActorLockFlag::TranslationZ);
							}));
						if (UI::PropertyCheckboxGroup("Z", translationZ))
						{
							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<RigidBodyComponent>();

								if (translationZ)
									component.LockFlags |= (uint8_t)ActorLockFlag::TranslationZ;
								else
									component.LockFlags &= ~(uint8_t)ActorLockFlag::TranslationZ;

								if (m_Context->IsPlaying())
								{
									auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
									actor->SetLockFlag(ActorLockFlag::TranslationZ, translationZ, true);
								}
							}
						}
						ImGui::PopItemFlag();
						UI::EndCheckboxGroup();

						UI::BeginCheckboxGroup("Freeze Rotation");
						ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<uint8_t, RigidBodyComponent>([](const RigidBodyComponent& other)
							{
								return (other.LockFlags & (uint8_t)ActorLockFlag::RotationX);
							}));
						if (UI::PropertyCheckboxGroup("X", rotationX))
						{
							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<RigidBodyComponent>();

								if (rotationX)
									component.LockFlags |= (uint8_t)ActorLockFlag::RotationX;
								else
									component.LockFlags &= ~(uint8_t)ActorLockFlag::RotationX;

								if (m_Context->IsPlaying())
								{
									auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
									actor->SetLockFlag(ActorLockFlag::RotationX, rotationX, true);
								}
							}
						}
						ImGui::PopItemFlag();

						ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<uint8_t, RigidBodyComponent>([](const RigidBodyComponent& other)
							{
								return (other.LockFlags & (uint8_t)ActorLockFlag::RotationY);
							}));
						if (UI::PropertyCheckboxGroup("Y", rotationY))
						{
							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<RigidBodyComponent>();

								if (rotationY)
									component.LockFlags |= (uint8_t)ActorLockFlag::RotationY;
								else
									component.LockFlags &= ~(uint8_t)ActorLockFlag::RotationY;

								if (m_Context->IsPlaying())
								{
									auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
									actor->SetLockFlag(ActorLockFlag::RotationY, rotationY, true);
								}
							}
						}
						ImGui::PopItemFlag();

						ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<uint8_t, RigidBodyComponent>([](const RigidBodyComponent& other)
							{
								return (other.LockFlags & (uint8_t)ActorLockFlag::RotationZ);
							}));

						if (UI::PropertyCheckboxGroup("Z", rotationZ))
						{
							for (auto& entityID : entities)
							{
								Entity entity = m_Context->GetEntityWithUUID(entityID);
								auto& component = entity.GetComponent<RigidBodyComponent>();

								if (rotationZ)
									component.LockFlags |= (uint8_t)ActorLockFlag::RotationZ;
								else
									component.LockFlags &= ~(uint8_t)ActorLockFlag::RotationZ;

								if (m_Context->IsPlaying())
								{
									auto actor = m_Context->GetPhysicsScene()->GetActor(entity);
									actor->SetLockFlag(ActorLockFlag::RotationZ, rotationZ, true);
								}
							}
						}
						ImGui::PopItemFlag();
						UI::EndCheckboxGroup();

						UI::EndPropertyGrid();
						UI::EndTreeNode();
					}
				}

				UI::EndPropertyGrid();
			}, EditorResources::RigidBodyIcon);

		DrawComponent<CharacterControllerComponent>("Character Controller", [&](CharacterControllerComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				// Layer has been removed, set to Default layer
				if (!PhysicsLayerManager::IsLayerValid(firstComponent.LayerID))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CharacterControllerComponent>().LayerID = 0;

						if (m_Context->IsPlaying())
						{
							auto controller = m_Context->GetPhysicsScene()->GetController(entity);
							controller->SetSimulationData(firstComponent.LayerID);
						}
					}
				}

				int layerCount = PhysicsLayerManager::GetLayerCount();
				const auto& layerNames = PhysicsLayerManager::GetLayerNames();
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<int, CharacterControllerComponent>([](const CharacterControllerComponent& other) { return other.LayerID; }));
				uint32_t oldLayer = firstComponent.LayerID;
				if (UI::PropertyDropdown("Layer", layerNames, layerCount, (int*)&firstComponent.LayerID))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CharacterControllerComponent>().LayerID = firstComponent.LayerID;

						if (m_Context->IsPlaying())
						{
							auto controller = m_Context->GetPhysicsScene()->GetController(entity);
							if (controller)
							{
								firstComponent.LayerID = controller->SetSimulationData(firstComponent.LayerID) ? firstComponent.LayerID : oldLayer;
							}
						}
					}
				}
				ImGui::PopItemFlag();

				auto physicsScene = m_Context->GetPhysicsScene();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, CharacterControllerComponent>([](const CharacterControllerComponent& other) { return other.DisableGravity; }));
				if (UI::Property("Disable Gravity", firstComponent.DisableGravity))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CharacterControllerComponent>().DisableGravity = firstComponent.DisableGravity;

						if (m_Context->IsPlaying())
						{
							auto controller = m_Context->GetPhysicsScene()->GetController(entity);
							if (controller)
								controller->SetGravityEnabled(!firstComponent.DisableGravity);
						}
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CharacterControllerComponent>([](const CharacterControllerComponent& other) { return other.SlopeLimitDeg; }));
				if (UI::Property("Slope Limit", firstComponent.SlopeLimitDeg, 1.0f, 0.0f, 90.0f))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CharacterControllerComponent>().SlopeLimitDeg = firstComponent.SlopeLimitDeg;

						if (m_Context->IsPlaying())
						{
							auto controller = physicsScene->GetController(entity);
							if (controller)
								controller->SetSlopeLimit(firstComponent.SlopeLimitDeg);
						}
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CharacterControllerComponent>([](const CharacterControllerComponent& other) { return other.StepOffset; }));
				if (UI::Property("Step Offset", firstComponent.StepOffset))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CharacterControllerComponent>().StepOffset = firstComponent.StepOffset;
						if (m_Context->IsPlaying())
						{
							auto controller = physicsScene->GetController(entity);
							if (controller)
								controller->SetStepOffset(firstComponent.StepOffset);
						}
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::CharacterControllerIcon);

		DrawComponent<FixedJointComponent>("Fixed Joint", [&](FixedJointComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				auto physicsScene = m_Context->GetPhysicsScene();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<UUID, FixedJointComponent>([](const FixedJointComponent& other) { return other.ConnectedEntity; }));
				if (UI::PropertyEntityReference("Connected Entity", firstComponent.ConnectedEntity, m_Context))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<FixedJointComponent>().ConnectedEntity = firstComponent.ConnectedEntity;

						if (m_Context->IsPlaying())
						{
							auto joint = physicsScene->GetJoint(entity);
							if (joint)
								joint->SetConnectedEntity(m_Context->TryGetEntityWithUUID(firstComponent.ConnectedEntity));
						}
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, FixedJointComponent>([](const FixedJointComponent& other) { return other.IsBreakable; }));
				if (UI::Property("Is Breakable", firstComponent.IsBreakable))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<FixedJointComponent>().IsBreakable = firstComponent.IsBreakable;

						if (m_Context->IsPlaying())
						{
							auto joint = physicsScene->GetJoint(entity);
							if (joint)
							{
								if (firstComponent.IsBreakable)
									joint->SetBreakForceAndTorque(firstComponent.BreakForce, firstComponent.BreakTorque);
								else
									joint->SetBreakForceAndTorque(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
							}
						}
					}
				}
				ImGui::PopItemFlag();

				if (firstComponent.IsBreakable)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, FixedJointComponent>([](const FixedJointComponent& other) { return other.BreakForce; }));
					if (UI::Property("Break Force", firstComponent.BreakForce, 1.0f))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<FixedJointComponent>().BreakForce = firstComponent.BreakForce;
							if (m_Context->IsPlaying())
							{
								auto joint = physicsScene->GetJoint(entity);
								if (joint)
									joint->SetBreakForceAndTorque(firstComponent.BreakForce, firstComponent.BreakTorque);
							}
						}
					}
					ImGui::PopItemFlag();
					UI::SetTooltip("The amount of force required to break this joint");

					ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, FixedJointComponent>([](const FixedJointComponent& other) { return other.BreakTorque; }));
					if (UI::Property("Break Torque", firstComponent.BreakTorque, 1.0f))
					{
						for (auto& entityID : entities)
						{
							Entity entity = m_Context->GetEntityWithUUID(entityID);
							entity.GetComponent<FixedJointComponent>().BreakTorque = firstComponent.BreakTorque;
							if (m_Context->IsPlaying())
							{
								auto joint = physicsScene->GetJoint(entity);
								if (joint)
									joint->SetBreakForceAndTorque(firstComponent.BreakForce, firstComponent.BreakTorque);
							}
						}
					}
					ImGui::PopItemFlag();
					UI::SetTooltip("The amount of torque required to break this joint");
				}

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, FixedJointComponent>([](const FixedJointComponent& other) { return other.EnableCollision; }));
				if (UI::Property("Enable Collision", firstComponent.EnableCollision))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<FixedJointComponent>().EnableCollision = firstComponent.EnableCollision;
						if (m_Context->IsPlaying())
						{
							auto joint = physicsScene->GetJoint(entity);
							if (joint)
								joint->SetCollisionEnabled(firstComponent.EnableCollision);
						}
					}
				}
				ImGui::PopItemFlag();
				UI::SetTooltip("Enable collision between the entities that this joint constraints");

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, FixedJointComponent>([](const FixedJointComponent& other) { return other.EnablePreProcessing; }));
				if (UI::Property("Enable Preprocessing", firstComponent.EnablePreProcessing))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<FixedJointComponent>().EnablePreProcessing = firstComponent.EnablePreProcessing;
						if (m_Context->IsPlaying())
						{
							auto joint = physicsScene->GetJoint(entity);
							if (joint)
								joint->SetPreProcessingEnabled(firstComponent.EnablePreProcessing);
						}
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::FixedJointIcon);

		DrawComponent<BoxColliderComponent>("Box Collider", [&](BoxColliderComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec3, BoxColliderComponent>([](const BoxColliderComponent& other) { return other.HalfSize; }));
				if (UI::Property("Half Size", firstComponent.HalfSize, 0.1f, 0.05f))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<BoxColliderComponent>().HalfSize = firstComponent.HalfSize;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec3, BoxColliderComponent>([](const BoxColliderComponent& other) { return other.Offset; }));
				if (UI::Property("Offset", firstComponent.Offset))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<BoxColliderComponent>().Offset = firstComponent.Offset;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, BoxColliderComponent>([](const BoxColliderComponent& other) { return other.IsTrigger; }));
				if (UI::Property("Is Trigger", firstComponent.IsTrigger))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<BoxColliderComponent>().IsTrigger = firstComponent.IsTrigger;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<AssetHandle, BoxColliderComponent>([](const BoxColliderComponent& other) { return other.Material; }));

				if (!AssetManager::IsAssetHandleValid(firstComponent.Material))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<BoxColliderComponent>().Material = 0;
					}
				}

				if (UI::PropertyAssetReference<PhysicsMaterial>("Material", firstComponent.Material))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<BoxColliderComponent>().Material = firstComponent.Material;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::BoxColliderIcon);

		DrawComponent<SphereColliderComponent>("Sphere Collider", [&](SphereColliderComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, SphereColliderComponent>([](const SphereColliderComponent& other) { return other.Radius; }));
				if (UI::Property("Radius", firstComponent.Radius))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SphereColliderComponent>().Radius = firstComponent.Radius;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec3, SphereColliderComponent>([](const SphereColliderComponent& other) { return other.Offset; }));
				if (UI::Property("Offset", firstComponent.Offset))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SphereColliderComponent>().Offset = firstComponent.Offset;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, SphereColliderComponent>([](const SphereColliderComponent& other) { return other.IsTrigger; }));
				if (UI::Property("Is Trigger", firstComponent.IsTrigger))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SphereColliderComponent>().IsTrigger = firstComponent.IsTrigger;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<AssetHandle, SphereColliderComponent>([](const SphereColliderComponent& other) { return other.Material; }));

				if (!AssetManager::IsAssetHandleValid(firstComponent.Material))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SphereColliderComponent>().Material = 0;
					}
				}

				if (UI::PropertyAssetReference<PhysicsMaterial>("Material", firstComponent.Material))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<SphereColliderComponent>().Material = firstComponent.Material;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::SphereColliderIcon);

		DrawComponent<CapsuleColliderComponent>("Capsule Collider", [&](CapsuleColliderComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CapsuleColliderComponent>([](const CapsuleColliderComponent& other) { return other.Radius; }));
				if (UI::Property("Radius", firstComponent.Radius))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CapsuleColliderComponent>().Radius = firstComponent.Radius;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, CapsuleColliderComponent>([](const CapsuleColliderComponent& other) { return other.Height; }));
				if (UI::Property("Height", firstComponent.Height))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CapsuleColliderComponent>().Height = firstComponent.Height;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<glm::vec3, CapsuleColliderComponent>([](const CapsuleColliderComponent& other) { return other.Offset; }));
				if (UI::Property("Offset", firstComponent.Offset))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CapsuleColliderComponent>().Offset = firstComponent.Offset;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, CapsuleColliderComponent>([](const CapsuleColliderComponent& other) { return other.IsTrigger; }));
				if (UI::Property("Is Trigger", firstComponent.IsTrigger))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CapsuleColliderComponent>().IsTrigger = firstComponent.IsTrigger;
					}
				}
				ImGui::PopItemFlag();

				if (!AssetManager::IsAssetHandleValid(firstComponent.Material))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CapsuleColliderComponent>().Material = 0;
					}
				}

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<AssetHandle, CapsuleColliderComponent>([](const CapsuleColliderComponent& other) { return other.Material; }));
				if (UI::PropertyAssetReference<PhysicsMaterial>("Material", firstComponent.Material))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<CapsuleColliderComponent>().Material = firstComponent.Material;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::CapsuleColliderIcon);

		DrawComponent<MeshColliderComponent>("Mesh Collider", [&](MeshColliderComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<AssetHandle, MeshColliderComponent>([](const MeshColliderComponent& other) { return other.ColliderAsset; }));
				if (UI::PropertyAssetReference<MeshColliderAsset>("Collider", firstComponent.ColliderAsset))
				{
					const auto& colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(firstComponent.ColliderAsset);

					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						auto& component = entity.GetComponent<MeshColliderComponent>();
						component.ColliderAsset = firstComponent.ColliderAsset;

						if (colliderAsset)
						{
							component.UseSharedShape = colliderAsset->AlwaysShareShape;
							component.CollisionComplexity = colliderAsset->CollisionComplexity;
						}

						if (component.ColliderAsset == 0)
							PhysicsSystem::GetOrCreateColliderAsset(entity, component);

						if (entity.HasComponent<MeshComponent>())
							component.SubmeshIndex = entity.GetComponent<MeshComponent>().SubmeshIndex;
					}
				}
				ImGui::PopItemFlag();

				auto& colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(firstComponent.ColliderAsset);
				const bool isPhysicalAsset = !AssetManager::IsMemoryAsset(firstComponent.ColliderAsset);

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, MeshColliderComponent>([](const MeshColliderComponent& other) { return other.IsTrigger; }));
				if (UI::Property("Is Trigger", firstComponent.IsTrigger))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<MeshColliderComponent>().IsTrigger = firstComponent.IsTrigger;
					}
				}
				ImGui::PopItemFlag();

				UI::BeginDisabled(colliderAsset && isPhysicalAsset);
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, MeshColliderComponent>([](const MeshColliderComponent& other) { return other.UseSharedShape; }));
				if (UI::Property("Use Shared Shape", firstComponent.UseSharedShape))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<MeshColliderComponent>().UseSharedShape = firstComponent.UseSharedShape;
					}
				}
				UI::SetTooltip("Allows this collider to share its collider data. (Default: False)");
				ImGui::PopItemFlag();
				UI::EndDisabled();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<AssetHandle, MeshColliderComponent>([](const MeshColliderComponent& other) { return other.OverrideMaterial; }));
				if (UI::PropertyAssetReference<PhysicsMaterial>("Material", firstComponent.OverrideMaterial))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<MeshColliderComponent>().OverrideMaterial = firstComponent.OverrideMaterial;
					}
				}
				ImGui::PopItemFlag();
				UI::SetTooltip("Overrides the material provided by the collider asset if an explicit asset has been set");

				UI::BeginDisabled(colliderAsset && isPhysicalAsset);
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<uint8_t, MeshColliderComponent>([](const MeshColliderComponent& other)
					{
						return (uint8_t)other.CollisionComplexity;
					}));
				static const char* s_ColliderUsageOptions[] = { "Default", "Use Complex as Simple", "Use Simple as Complex" };
				if (UI::PropertyDropdown<ECollisionComplexity, uint8_t>("Collision Complexity", s_ColliderUsageOptions, 3, firstComponent.CollisionComplexity))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						auto& mcc = entity.GetComponent<MeshColliderComponent>();
						mcc.CollisionComplexity = firstComponent.CollisionComplexity;

						auto collider = AssetManager::GetAsset<MeshColliderAsset>(mcc.ColliderAsset);
						if (collider)
							collider->CollisionComplexity = mcc.CollisionComplexity;
					}
				}
				ImGui::PopItemFlag();
				UI::EndDisabled();
				UI::EndPropertyGrid();

				if (UI::Button("Force cook mesh"))
					CookingFactory::CookMesh(firstComponent.ColliderAsset, true);
			}, EditorResources::MeshColliderIcon);

		DrawComponent<AudioListenerComponent>("Audio Listener", [&](AudioListenerComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				UI::BeginPropertyGrid();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, AudioListenerComponent>([](const AudioListenerComponent& other) { return other.Active; }));
				if (UI::Property("Active", firstComponent.Active))
				{
					auto view = m_Context->GetAllEntitiesWith<AudioListenerComponent>();
					if (firstComponent.Active == true)
					{
						for (auto ent : view)
						{
							Entity e{ ent, m_Context.Raw() };
							e.GetComponent<AudioListenerComponent>().Active = false;
						}

						firstComponent.Active = true;
					}
					else
					{
						// Fallback to using main camera as active listener
						// - in editor main camera is already the only allowed active listener (may change that in the future)
						// - in runtime it falls back to main camera in update loop if can't find other active listener
					}
				}
				ImGui::PopItemFlag();

				float inAngle = glm::degrees(firstComponent.ConeInnerAngleInRadians);
				float outAngle = glm::degrees(firstComponent.ConeOuterAngleInRadians);
				float outGain = firstComponent.ConeOuterGain;
				//? Have to manually clamp here because UI::Property doesn't take flags to pass in ImGuiSliderFlags_ClampOnInput

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, AudioListenerComponent>([](const AudioListenerComponent& other) { return glm::degrees(other.ConeInnerAngleInRadians); }));
				if (UI::Property("Inner Cone Angle", inAngle, 1.0f, 0.0f, 360.0f))
				{
					if (inAngle > 360.0f) inAngle = 360.0f;
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<AudioListenerComponent>().ConeInnerAngleInRadians = glm::radians(inAngle);
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, AudioListenerComponent>([](const AudioListenerComponent& other) { return glm::degrees(other.ConeOuterAngleInRadians); }));
				if (UI::Property("Outer Cone Angle", outAngle, 1.0f, 0.0f, 360.0f))
				{
					if (outAngle > 360.0f) outAngle = 360.0f;
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<AudioListenerComponent>().ConeOuterAngleInRadians = glm::radians(outAngle);
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, AudioListenerComponent>([](const AudioListenerComponent& other) { return other.ConeOuterGain; }));
				if (UI::Property("Outer Gain", outGain, 0.01f, 0.0f, 1.0f))
				{
					if (outGain > 1.0f) outGain = 1.0f;
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<AudioListenerComponent>().ConeOuterGain = outGain;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
			}, EditorResources::AudioListenerIcon);

		DrawComponent<AudioComponent>("Audio", [&](AudioComponent& firstComponent, const std::vector<UUID>& entities, const bool isMultiEdit)
			{
				// PropertyGrid consists out of 2 columns, so need to move cursor accordingly
				auto propertyGridSpacing = []
				{
					ImGui::Spacing();
					ImGui::NextColumn();
					ImGui::NextColumn();
				};

				// Making separators a little bit less bright to "separate" them visually from the text
				auto& colors = ImGui::GetStyle().Colors;
				auto oldSCol = colors[ImGuiCol_Separator];
				const float brM = 0.6f;
				colors[ImGuiCol_Separator] = ImVec4{ oldSCol.x * brM, oldSCol.y * brM, oldSCol.z * brM, 1.0f };

				//=======================================================

				//--- Sound Assets and Looping
				//----------------------------
				UI::PushID();
				UI::BeginPropertyGrid();
				// Need to wrap this first Property Grid into another ID,
				// otherwise there's a conflict with the next Property Grid.

				//=== Audio Objects API

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentString<AudioComponent>([](const AudioComponent& other) { return other.StartEvent; }));
				if (UI::Property("Start Event", firstComponent.StartEvent))
				{
					firstComponent.StartCommandID = Audio::CommandID::FromString(firstComponent.StartEvent.c_str());

					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						auto& component = entity.GetComponent<AudioComponent>();
						component.StartEvent = firstComponent.StartEvent;
						component.StartCommandID = firstComponent.StartCommandID;
					}
				}
				ImGui::PopItemFlag();

				//=====================

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, AudioComponent>([](const AudioComponent& other) { return other.VolumeMultiplier; }));
				if (UI::Property("Volume Multiplier", firstComponent.VolumeMultiplier, 0.01f, 0.0f, 1.0f)) //TODO: switch to dBs in the future ?
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<AudioComponent>().VolumeMultiplier = firstComponent.VolumeMultiplier;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<float, AudioComponent>([](const AudioComponent& other) { return other.PitchMultiplier; }));
				if (UI::Property("Pitch Multiplier", firstComponent.PitchMultiplier, 0.01f, 0.0f, 24.0f)) // max pitch 24 is just an arbitrary number here
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<AudioComponent>().PitchMultiplier = firstComponent.PitchMultiplier;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, AudioComponent>([](const AudioComponent& other) { return other.bPlayOnAwake; }));
				if (UI::Property("Play on Awake", firstComponent.bPlayOnAwake))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<AudioComponent>().bPlayOnAwake = firstComponent.bPlayOnAwake;
					}
				}
				ImGui::PopItemFlag();

				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, isMultiEdit && IsInconsistentPrimitive<bool, AudioComponent>([](const AudioComponent& other) { return other.bStopWhenEntityDestroyed; }));
				if (UI::Property("Stop When Entity Is Destroyed", firstComponent.bStopWhenEntityDestroyed))
				{
					for (auto& entityID : entities)
					{
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.GetComponent<AudioComponent>().bStopWhenEntityDestroyed = firstComponent.bStopWhenEntityDestroyed;
					}
				}
				ImGui::PopItemFlag();

				UI::EndPropertyGrid();
				UI::PopID();

				colors[ImGuiCol_Separator] = oldSCol;
			}, EditorResources::AudioIcon);
	}

	void SceneHierarchyPanel::OnExternalEntityDestroyed(Entity entity)
	{
		m_EntityDeletedCallback(entity);
	}

}