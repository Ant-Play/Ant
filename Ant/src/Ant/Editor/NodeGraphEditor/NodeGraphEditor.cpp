#include "antpch.h"
#include "NodeGraphEditor.h"

#include "NodeGraphAsset.h"
#include "NodeGraphEditorContext.h"
#include "NodeEditorModel.h"

#include "Ant/Asset/AssetManager.h"
#include "Ant/Core/Inputs.h"
#include "Ant/Editor/EditorResources.h"
#include "Ant/ImGui/ImGuiUtilities.h"
#include "Ant/ImGui/Colours.h"
#include "Ant/Vendor/imgui-node-editor/widgets.h"

#include "choc/text/choc_StringUtilities.h"
#include "choc/text/choc_FloatToString.h"
#include "imgui-node-editor/builders.h"
#include "imgui-node-editor/imgui_node_editor_internal.h"

namespace ed = ax::NodeEditor;
namespace utils = ax::NodeEditor::Utilities;

static ax::NodeEditor::Detail::EditorContext* GetEditorContext()
{
	return (ax::NodeEditor::Detail::EditorContext*)(ed::GetCurrentEditor());
}

namespace Ant
{
	//==================================================================================
	float GetBestWidthForEnumCombo(const std::vector<Token>& enumTokens)
	{
		float width = 40.0f;
		for (const auto& token : enumTokens)
		{
			const float strWidth = ImGui::CalcTextSize(token.name.data()).x;
			if (strWidth > width)
				width = strWidth;
		}
		return width + GImGui->Style.WindowPadding.x * 2.0f;
	}


	//==================================================================================
	/// AssetHandle choc::Value utilities
	uint64_t NodeGraphEditorBase::GetAssetHandleValue(const choc::value::ValueView& v)
	{
		ANT_CORE_ASSERT(IsAssetHandle(v));

		// Assuming asset handle value stored as int64 [Data]["Value"] member in choc Value object
		return (uint64_t)(v.isInt64() ? v.getInt64() : v["Value"].getInt64());
	}

	void NodeGraphEditorBase::SetAssetHandleValue(choc::value::Value& v, uint64_t value)
	{
		v = ValueFrom(UUID(value));
	}

	void NodeGraphEditorBase::SetAssetHandleValue(choc::value::ValueView& v, uint64_t value)
	{
		if (v.isInt64())
		{
			ANT_CORE_ERROR_TAG("SetAssetHandleValue", "Using old API - storing AssetHandle as int64 Value instead of Value Class Object.");
			v.set((int64_t)value);
		}
		else if (v.hasObjectMember("Value"))
		{
			choc::value::ValueView& data = v["Value"];
			if (data.isInt64())
				data.set((int64_t)value);
		}
	}

	//==================================================================================
	void NodeGraphEditorBase::InitializeEditorStyle(ax::NodeEditor::Style& editorStyle)
	{
		// Style
		editorStyle.NodePadding = { 0.0f, 4.0f, 0.0f, 0.0f }; // This mostly affects Comment nodes
		editorStyle.NodeRounding = 3.0f;
		editorStyle.NodeBorderWidth = 1.0f;
		editorStyle.HoveredNodeBorderWidth = 2.0f;
		editorStyle.SelectedNodeBorderWidth = 3.0f;
		editorStyle.PinRounding = 2.0f;
		editorStyle.PinBorderWidth = 0.0f;
		editorStyle.LinkStrength = 80.0f;
		editorStyle.ScrollDuration = 0.35f;
		editorStyle.FlowMarkerDistance = 30.0f;
		editorStyle.FlowDuration = 2.0f;
		editorStyle.GroupRounding = 0.0f;
		editorStyle.GroupBorderWidth = 0.0f;
		editorStyle.GridSnap = 8.0f;

		editorStyle.HighlightConnectedLinks = 1.0f;

		// Extra (for now just using defaults)
		editorStyle.SnapLinkToPinDir = 0.0f;
		editorStyle.PivotAlignment = ImVec2(0.5f, 0.5f);    // This one is changed in Draw
		editorStyle.PivotSize = ImVec2(0.0f, 0.0f);         // This one is changed in Draw
		editorStyle.PivotScale = ImVec2(1, 1);              // This one is not used
		editorStyle.PinCorners = ImDrawFlags_RoundCornersAll;
		editorStyle.PinRadius = 0.0f;       // This one is changed in Draw
		editorStyle.PinArrowSize = 0.0f;    // This one is changed in Draw
		editorStyle.PinArrowWidth = 0.0f;   // This one is changed in Draw

		// Colours
		editorStyle.Colors[ed::StyleColor_Bg] = ImColor(23, 24, 28, 200);
		editorStyle.Colors[ed::StyleColor_Grid] = ImColor(21, 21, 21, 255);// ImColor(60, 60, 60, 40);
		editorStyle.Colors[ed::StyleColor_NodeBg] = ImColor(31, 33, 38, 255);
		editorStyle.Colors[ed::StyleColor_NodeBorder] = ImColor(51, 54, 62, 140);
		editorStyle.Colors[ed::StyleColor_HovNodeBorder] = ImColor(60, 180, 255, 150);
		editorStyle.Colors[ed::StyleColor_SelNodeBorder] = ImColor(255, 225, 135, 255);
		editorStyle.Colors[ed::StyleColor_NodeSelRect] = ImColor(5, 130, 255, 64);
		editorStyle.Colors[ed::StyleColor_NodeSelRectBorder] = ImColor(5, 130, 255, 128);
		editorStyle.Colors[ed::StyleColor_HovLinkBorder] = ImColor(60, 180, 255, 255);
		editorStyle.Colors[ed::StyleColor_SelLinkBorder] = ImColor(255, 225, 135, 255);
		editorStyle.Colors[ed::StyleColor_LinkSelRect] = ImColor(5, 130, 255, 64);
		editorStyle.Colors[ed::StyleColor_LinkSelRectBorder] = ImColor(5, 130, 255, 128);
		editorStyle.Colors[ed::StyleColor_PinRect] = ImColor(60, 180, 255, 0);
		editorStyle.Colors[ed::StyleColor_PinRectBorder] = ImColor(60, 180, 255, 0);
		editorStyle.Colors[ed::StyleColor_Flow] = ImColor(255, 128, 64, 255);
		editorStyle.Colors[ed::StyleColor_FlowMarker] = ImColor(255, 128, 64, 255);
		editorStyle.Colors[ed::StyleColor_GroupBg] = ImColor(255, 255, 255, 30);
		editorStyle.Colors[ed::StyleColor_GroupBorder] = ImColor(0, 0, 0, 0);

		editorStyle.Colors[ed::StyleColor_HighlightLinkBorder] = ImColor(255, 255, 255, 140);
	}

	NodeGraphEditorBase::NodeGraphEditorBase(const char* id)
		: AssetEditor(id)
	{
		m_State = std::make_unique<ContextState>();
	}

	void NodeGraphEditorBase::OnOpen()
	{
	}

	void NodeGraphEditorBase::OnClose()
	{
		GetModel()->SaveAll();

		if (m_Editor)
		{
			ed::DestroyEditor(m_Editor);
			m_Editor = nullptr;
		}

		m_GraphAsset = nullptr;
	}

	void NodeGraphEditorBase::SetAsset(const Ref<Asset>& asset)
	{
		m_GraphAsset = asset;
	}

	std::vector<UUID> NodeGraphEditorBase::GetSelectedNodes() const
	{
		std::vector<ed::NodeId> selectedNodes;
		selectedNodes.resize(ed::GetSelectedObjectCount());
		int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
		selectedNodes.resize(nodeCount);

		std::vector<UUID> ids;
		for (const auto& nodeID : selectedNodes)
			ids.push_back(nodeID.Get());

		return ids;
	}


	bool NodeGraphEditorBase::InitializeEditor()
	{
		if (!m_GraphAsset)
			return false;

		ed::Config config;
		config.SettingsFile = nullptr;
		config.UserPointer = this;

		// Set up custom zeoom levels
		{
			// Limitting max zoom in to 1:1
			static constexpr float zooms[] = { 0.1f, 0.15f, 0.20f, 0.25f, 0.33f, 0.5f, 0.75f, 1.0f };
			for (uint32_t i = 0; i < sizeof(zooms) / sizeof(*zooms); ++i)
				config.CustomZoomLevels.push_back(zooms[i]);
		}

		// Save graph state
		config.SaveSettings = [](const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
		{
			auto* nodeEditor = static_cast<NodeGraphEditorBase*>(userPointer);
			return nodeEditor->GetModel()->SaveGraphState(data, size);
		};

		// Load graph state
		config.LoadSettings = [](char* data, void* userPointer) -> size_t
		{
			auto* nodeEditor = static_cast<NodeGraphEditorBase*>(userPointer);
			const std::string& graphState = nodeEditor->GetModel()->LoadGraphState();
			if (data)
			{
				memcpy(data, graphState.data(), graphState.size());
			}
			return graphState.size();
		};

		// Restore node location
		config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
		{
			auto* nodeEditor = static_cast<NodeGraphEditorBase*>(userPointer);

			if (!nodeEditor->GetModel())
				return 0;

			auto node = nodeEditor->GetModel()->FindNode(UUID(nodeId.Get()));
			if (!node)
				return 0;


			if (data != nullptr)
				memcpy(data, node->State.data(), node->State.size());
			return node->State.size();
		};

		// Store node location
		config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
		{
			auto* nodeEditor = static_cast<NodeGraphEditorBase*>(userPointer);

			if (!nodeEditor->GetModel())
				return false;

			auto node = nodeEditor->GetModel()->FindNode(UUID(nodeId.Get()));
			if (!node)
				return false;

			node->State.assign(data, size);

			////self->TouchNode(nodeId);

			return true;
		};

		m_Editor = ed::CreateEditor(&config);
		ed::SetCurrentEditor(m_Editor);

		InitializeEditorStyle(ed::GetStyle());

		return true;
	}

	void NodeGraphEditorBase::DrawNodeContextMenu(Node* node)
	{
		ImGui::TextUnformatted("Node Context Menu");
		ImGui::Separator();
		if (node)
		{
			ImGui::Text("ID: %s", std::to_string(node->ID).c_str());
			ImGui::Text("Sort intex: %d", node->SortIndex);
			ImGui::Text("Inputs: %d", (int)node->Inputs.size());
			ImGui::Text("Outputs: %d", (int)node->Outputs.size());
		}
		else
			ImGui::Text("Unknown node: %p", m_State->ContextNodeId.AsPointer());
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
			ed::DeleteNode(m_State->ContextNodeId);
	}

	void NodeGraphEditorBase::DrawPinContextMenu(Pin* pin)
	{
		// TODO: move this to Utils header
		auto GetTypeName = [](const choc::value::Value& v) -> std::string
		{
			bool isArray = v.isArray();
			choc::value::Type type = isArray ? v.getType().getElementType() : v.getType();

			if (type.isFloat())	return "Float";
			else if (type.isInt32())	return "Int";
			else if (type.isBool())		return "Bool";
			else if (type.isString())	return "String";
			else if (IsAssetHandle(type))  return "AssetHandle";
			else if (type.isObject())
			{
				if (isArray && v[0].hasObjectMember("TypeName"))
					return v[0]["TypeName"].get<std::string>();
				else if (v.hasObjectMember("TypeName"))
					return v["TypeName"].get<std::string>();
				else if (v.hasObjectMember("Type"))
					return v["Type"].get<std::string>();
				if (isArray && v[0].hasObjectMember("Type"))
					return v[0]["Type"].get<std::string>();
			}

			return "invalid";
		};

		ImGui::TextUnformatted("Pin Context Menu");
		ImGui::Separator();
		if (pin)
		{
			ImGui::Text("ID: %s", std::to_string(pin->ID).c_str());
			if (pin->NodeID)
			{
				ImGui::Text("Node: %s", std::to_string(pin->NodeID).c_str());
			}
			else
				ImGui::Text("Node: %s", "<none>");

			ImGui::Text("Value type: %s", GetTypeName(pin->Value).c_str());
		}
		else
			ImGui::Text("Unknown pin: %s", m_State->ContextPinId.AsPointer());
	}

	void NodeGraphEditorBase::DrawLinkContextMenu(Link* link)
	{
		ImGui::TextUnformatted("Link Context Menu");
		ImGui::Separator();
		if (link)
		{
			ImGui::Text("ID: %s", std::to_string(link->ID).c_str());
			ImGui::Text("From: %s", std::to_string(link->StartPinID).c_str());
			ImGui::Text("To: %s", std::to_string(link->EndPinID).c_str());
		}
		else
			ImGui::Text("Unknown link: %p", m_State->ContextLinkId.AsPointer());
		ImGui::Separator();
		if (ImGui::MenuItem("Delete"))
			ed::DeleteLink(m_State->ContextLinkId);
	}

	static AssetHandle s_AssetComboSelectedHandle = 0;
	static Node* s_AssetComboNode = nullptr;
	static Pin* s_AssetComboPin = nullptr;
	static ImVec2 s_AssetComboStart;
	static ImVec2 s_AssetComboSize = ImVec2(0.0f, 280.0f);
	static bool s_ShouldAssetComboOpen = false;
	static const char* s_AssetComboPopupID = "AssetPinCombo";

	struct SelectAssetPinContext
	{
		UUID PinID = 0;
		AssetHandle SelectedAssetHandle = 0;
		std::string AssetType;
	};

	static SelectAssetPinContext s_SelectAssetContext;

	struct EnumPinContext
	{
		UUID PinID = 0;
		int SelectedValue = 0;

		/*	Implementation might want to store enum value as a member of an object, or as int32.
			This callback allows implementation to construct choc::Value from selected item on change.
		*/
		std::function<choc::value::Value(int selected)> ConstructValue;
	};

	static EnumPinContext s_SelectedEnumContext;

	void NodeGraphEditorBase::Render()
	{
		if (!GetModel())
			return;

		if (!m_Editor || !m_Initialized)
		{
			m_Initialized = InitializeEditor();
		}

		// Main Docking Space
		//-------------------
		ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_AutoHideTabBar;
		m_MainDockID = ImGui::DockSpace(ImGui::GetID("NodeEditor"), ImVec2(0, 0), dockFlags);

		UI::ScopedStyle stylePadding(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 2.0f));

		// TODO: this might have to be unique ID of the asset to prevend docking with other graph windows
		m_WindowClass.ClassId = ImGui::GetID("NodeEditor");
		m_WindowClass.DockingAllowUnclassed = false;

		ImGui::SetNextWindowDockID(m_MainDockID, ImGuiCond_Always);
		ImGui::SetNextWindowClass(GetWindowClass());

		ImGui::Begin("Canvas", nullptr, ImGuiWindowFlags_NoCollapse);

		// This ensures that we don't start selection rectangle when dragging Canvas window titlebar
		bool draggingnCanvasTitlebar = false;
		if (ImGui::IsItemActive())
			draggingnCanvasTitlebar = true;


		ed::SetCurrentEditor(m_Editor);

		ed::NodeId& contextNodeId = m_State->ContextNodeId;
		ed::LinkId& contextLinkId = m_State->ContextLinkId;
		ed::PinId& contextPinId = m_State->ContextPinId;
		auto& createNewNode = m_State->CreateNewNode;
		auto& newNodeLinkPinId = m_State->NewNodeLinkPinId;
		auto& newLinkPin = m_State->NewLinkPin;

		// Set to 'true' to prevent editor from creating selection rectangle
		// when draggin node input field
		auto& draggingInputField = m_State->DraggingInputField;

		static bool s_NewNodePopupOpening = false;

		ed::Begin("Node editor");
		{
			auto cursorTopLeft = ImGui::GetCursorScreenPos();

			DrawNodes();

			// Links
			for (auto& link : GetModel()->GetLinks())
				ed::Link(ed::LinkId(link.ID), ed::PinId(link.StartPinID), ed::PinId(link.EndPinID), link.Color, 2.0f);

			m_AcceptingLinkPins = { nullptr, nullptr };

			// Draw dragging Link
			if (!createNewNode)
			{
				const ImColor draggedLinkColour = ImColor(255, 255, 255);
				const float lineThickness = 2.0f;
				if (ed::BeginCreate(draggedLinkColour, lineThickness))
				{
					auto showLabel = [](const char* label, ImColor color)
					{
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
						auto size = ImGui::CalcTextSize(label);

						auto padding = ImGui::GetStyle().FramePadding;
						auto spacing = ImGui::GetStyle().ItemSpacing;

						ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

						auto rectMin = ImGui::GetCursorScreenPos() - padding;
						auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

						auto drawList = ImGui::GetWindowDrawList();
						drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
						ImGui::TextUnformatted(label);
					};

					ed::PinId startPinId = 0, endPinId = 0;
					if (ed::QueryNewLink(&startPinId, &endPinId))
					{
						auto startPin = GetModel()->FindPin(startPinId.Get());
						auto endPin = GetModel()->FindPin(endPinId.Get());

						newLinkPin = startPin ? startPin : endPin;

						if (startPin->Kind == PinKind::Input)
						{
							std::swap(startPin, endPin);
							std::swap(startPinId, endPinId);
						}

						if (startPin && endPin)
						{
							NodeEditorModel::LinkQueryResult canConnect = GetModel()->CanCreateLink(startPin, endPin);
							if (canConnect)
							{
								m_AcceptingLinkPins.first = startPin;
								m_AcceptingLinkPins.second = endPin;
								showLabel("+ Create Link", ImColor(32, 45, 32, 180));

								// Set link colour when attacking to a compatible pin
								const ImColor linkColor = GetIconColor(startPin->GetType());

								if (ed::AcceptNewItem(linkColor, 4.0f))
								{
									GetModel()->CreateLink(startPin, endPin);
								}
							}
							else
							{
								switch (canConnect.Reason)
								{
								case NodeEditorModel::LinkQueryResult::IncompatibleStorageKind:
								case NodeEditorModel::LinkQueryResult::IncompatiblePinKind:
								{
									showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
									ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
									break;
								}
								case NodeEditorModel::LinkQueryResult::IncompatibleType:
								{
									showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
									ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
									break;
								}
								case NodeEditorModel::LinkQueryResult::SamePin:
								case NodeEditorModel::LinkQueryResult::SameNode:
								{
									showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
									ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
									break;
								}
								case NodeEditorModel::LinkQueryResult::CausesLoop:
								{
									showLabel("x Connection causes loop", ImColor(45, 32, 32, 180));
									ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
									break;
								}
								case NodeEditorModel::LinkQueryResult::InvalidStartPin:
								case NodeEditorModel::LinkQueryResult::InvalidEndPin:
								default:
									break;
								}
							}
						}
					}

					ed::PinId pinId = 0;
					if (ed::QueryNewNode(&pinId))
					{
						newLinkPin = GetModel()->FindPin(pinId.Get());
						if (newLinkPin)
							showLabel("+ Create Node", ImColor(32, 45, 32, 180));

						const ImColor draggedLinkColour = newLinkPin ? GetIconColor(newLinkPin->GetType()) : ImColor(255, 255, 255);
						const float lineThickness = 2.0f;

						if (ed::AcceptNewItem(draggedLinkColour, lineThickness))
						{
							createNewNode = true;
							newNodeLinkPinId = pinId.Get();
							newLinkPin = nullptr;

							ed::Suspend();
							ImGui::OpenPopup("Create New Node");
							s_NewNodePopupOpening = true;
							ed::Resume();
						}
					}
				}
				else
					newLinkPin = nullptr;

				ed::EndCreate();

				if (ed::BeginDelete())
				{
					ed::LinkId linkId = 0;
					while (ed::QueryDeletedLink(&linkId))
					{
						if (ed::AcceptDeletedItem())
							GetModel()->RemoveLink(linkId.Get());
					}

					ed::NodeId nodeId = 0;
					while (ed::QueryDeletedNode(&nodeId))
					{
						if (ed::AcceptDeletedItem())
							GetModel()->RemoveNode(nodeId.Get());
					}
				}
				ed::EndDelete();
			}

			// Alt + Click to remove links connected to clicked pin
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::GetIO().KeyAlt)
			{
				auto hoveredPinId = (UUID)ed::GetHoveredPin().Get();
				if (hoveredPinId)
				{
					auto* model = GetModel();
					std::vector<UUID> links = model->GetAllLinkIDsConnectedToPin(hoveredPinId);
					model->RemoveLinks(links);
				}
			}

			ImGui::SetCursorScreenPos(cursorTopLeft);

			ed::Suspend();

			if (ed::ShowNodeContextMenu(&contextNodeId))
				ImGui::OpenPopup("Node Context Menu");
			else if (ed::ShowPinContextMenu(&contextPinId))
				ImGui::OpenPopup("Pin Context Menu");
			else if (ed::ShowLinkContextMenu(&contextLinkId))
				ImGui::OpenPopup("Link Context Menu");
			else if (ed::ShowBackgroundContextMenu())
			{
				ImGui::OpenPopup("Create New Node");
				s_NewNodePopupOpening = true;
				newNodeLinkPinId = 0;
			}
			ed::Resume();
			ed::Suspend();

			UI::ScopedStyle windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
			UI::ScopedStyle disableBorder(ImGuiStyleVar_FrameBorderSize, 0.0f);
			UI::ScopedColourStack popupColours(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 80),
				ImGuiCol_Separator, IM_COL32(90, 90, 90, 255),
				ImGuiCol_Text, Colours::Theme::textBrighter);

			if (UI::BeginPopup("Node Context Menu"))
			{
				auto node = GetModel()->FindNode(contextNodeId.Get());
				DrawNodeContextMenu(node);

				UI::EndPopup();
			}

			if (UI::BeginPopup("Pin Context Menu"))
			{
				auto pin = GetModel()->FindPin(contextPinId.Get());
				DrawPinContextMenu(pin);

				UI::EndPopup();
			}

			if (UI::BeginPopup("Link Context Menu"))
			{
				auto link = GetModel()->FindLink(contextLinkId.Get());
				DrawLinkContextMenu(link);

				UI::EndPopup();
			}

			bool isNewNodePopuped = false;

			static ImVec2 newNodePostion{ 0.0f, 0.0f };

			{
				UI::ScopedStyle scrollListStyle(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));

				if (UI::BeginPopup("Create New Node"))
				{
					isNewNodePopuped = true;

					newNodePostion = ed::ScreenToCanvas(ImGui::GetMousePosOnOpeningCurrentPopup());

					Node* node = nullptr;

					// Search widget
					static bool grabFocus = true;
					static std::string searchString;
					if (ImGui::GetCurrentWindow()->Appearing)
					{
						grabFocus = true;
						searchString.clear();
					}
					UI::ShiftCursorX(4.0f);
					UI::ShiftCursorY(2.0f);
					UI::Widgets::SearchWidget(searchString, "Search nodes...", &grabFocus);
					const bool searching = !searchString.empty();

					{
						UI::ScopedColourStack headerColours(ImGuiCol_Header, IM_COL32(255, 255, 255, 0),
							ImGuiCol_HeaderActive, IM_COL32(45, 46, 51, 255),
							ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 80));

						//UI::ScopedColour scrollList(ImGuiCol_ChildBg, IM_COL32(255, 255, 255, 0));
						//UI::ScopedStyle scrollListStyle(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
						UI::ShiftCursorY(GImGui->Style.WindowPadding.y + 2.0f);

						if (ImGui::BeginChild("##new_node_scroll_list", ImVec2(0.0f, 350.0f)))
						{
							// If subclass graph editor has any custom nodes,
							// the popup items for them can be added here
							if (onNodeListPopup)
								node = onNodeListPopup(searching, searchString);

							const auto& nodeRegistry = GetModel()->GetNodeTypes();

							for (const auto& [categoryName, category] : nodeRegistry)
							{

								// Can use this instead of the collapsing header
								//UI::PopupMenuHeader(categoryName, true, false);

								if (searching)
									ImGui::SetNextItemOpen(true);

								ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(140, 140, 140, 255));
								if (UI::ContextMenuHeader(categoryName.c_str()))
								{
									ImGui::PopStyleColor(); // header Text
									//ImGui::Separator();

									ImGui::Indent();
									//-------------------------------------------------
									if (searching)
									{
										for (const auto& [nodeName, spawnFunction] : category)
										{
											const std::string nameNoUnderscores = choc::text::replace(nodeName, "_", " ");

											if (UI::IsMatchingSearch(categoryName, searchString)
												|| UI::IsMatchingSearch(nodeName, searchString))
											{
												if (ImGui::MenuItem(nameNoUnderscores.c_str()))
													node = GetModel()->CreateNode(categoryName, nodeName);

											}
										}
									}
									else
									{
										for (const auto& [nodeName, spawnFunction] : category)
										{
											if (ImGui::MenuItem(choc::text::replace(nodeName, "_", " ").c_str()))
												node = GetModel()->CreateNode(categoryName, nodeName);
										}
									}

									if (nodeRegistry.find(categoryName) != nodeRegistry.end())
										ImGui::Spacing();
									ImGui::Unindent();
								}
								else
								{
									ImGui::PopStyleColor(); // header Text
								}
							}
						}
						ImGui::EndChild();
					}

					if (node)
					{
						createNewNode = false;

						ed::SetNodePosition(ed::NodeId(node->ID), newNodePostion);

						if (auto* startPin = GetModel()->FindPin(newNodeLinkPinId))
						{
							auto& pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

							ANT_CORE_ASSERT(!pins.empty());

							for (auto& pin : pins)
							{
								if (GetModel()->CanCreateLink(startPin, pin))
								{
									auto endPin = pin;
									if (startPin->Kind == PinKind::Input)
										std::swap(startPin, endPin);

									GetModel()->CreateLink(startPin, endPin);

									break;
								}
							}
						}

						ImGui::CloseCurrentPopup();
					}
					auto* popupWindow = ImGui::GetCurrentWindow();
					auto shadowRect = ImRect(popupWindow->Pos, popupWindow->Pos + popupWindow->Size);

					s_NewNodePopupOpening = false;

					UI::EndPopup();
					UI::DrawShadow(EditorResources::ShadowTexture, 14, shadowRect, 1.3f);

				}
				else
					createNewNode = false;
			}

			ed::Resume();

			if (isNewNodePopuped && newNodeLinkPinId)
			{
				if (const auto* newNodeLinkPin = GetModel()->FindPin(newNodeLinkPinId))
				{
					const float lineThickness = 2.0f;
					auto& creator = GetEditorContext()->GetItemCreator();
					if (creator.m_IsActive)
						creator.SetStyle(GetIconColor(newNodeLinkPin->GetType()), lineThickness);

					// this is needed to fix the case when if you move mouse fast when release dragged link,
					// the link end droppes along the way behind the actual popup position
					auto& draggedPivot = newNodeLinkPin->Kind == PinKind::Output
						? creator.m_lastEndPivot
						: creator.m_lastStartPivot;

					const ImVec2 offset = newNodePostion - draggedPivot.GetCenter();
					draggedPivot.Translate(offset);

					ed::DrawLastLink();
				}
			}
		}

		//? This is a bit ugly, but works. Should be changed later and moved to DrawNodes()
		if (s_AssetComboPin != nullptr)
		{
			ed::Suspend();

			if (s_ShouldAssetComboOpen && !ImGui::IsPopupOpen(s_AssetComboPopupID))
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MousePosPrev = io.MousePos;
				io.MousePos = s_AssetComboStart;
				ImGui::OpenPopup(s_AssetComboPopupID);
				io.MousePos = io.MousePosPrev;

				s_ShouldAssetComboOpen = false;
			}

			Ref<AudioFile> asset;
			if (UI::PropertyAssetDropdown(s_AssetComboPopupID, asset, s_AssetComboSize, &s_AssetComboSelectedHandle))
			{
				//ANT_CORE_INFO("Asset selected");

				SetAssetHandleValue(s_AssetComboPin->Value, s_AssetComboSelectedHandle);

				if (GetModel()->onPinValueChanged)
				{
					GetModel()->onPinValueChanged(s_AssetComboNode->ID, s_AssetComboPin->ID);
				}

				s_AssetComboNode = nullptr;
				s_AssetComboPin = nullptr;
				s_AssetComboSelectedHandle = 0;
				ImGui::CloseCurrentPopup();
			}

			ed::Resume();
		}

		ed::End();
		auto editorMin = ImGui::GetItemRectMin();
		auto editorMax = ImGui::GetItemRectMax();

		// Draw current zoom level
		{
			UI::ScopedFont largeFont(ImGui::GetIO().Fonts->Fonts[1]);
			const ImVec2 zoomLabelPos({ editorMin.x + 20.0f, editorMax.y - 40.0f });
			const std::string zoomLabelText = "Zoom " + choc::text::floatToString(ed::GetCurrentZoom(), 1, true);

			ImGui::GetWindowDrawList()->AddText(zoomLabelPos, IM_COL32(255, 255, 255, 60), zoomLabelText.c_str());
		}

		UI::DrawShadowInner(EditorResources::ShadowTexture, 50, editorMin, editorMax, 0.3f, (editorMax.y - editorMin.y) / 4.0f);

		// A big of a hack to prevent editor from dragging and selecting while dragging an input field
		// It still draws a dot where the Selection Action should be, but it's not too bad
		if (draggingInputField || draggingnCanvasTitlebar)
		{
			auto* editor = GetEditorContext();
			if (auto* action = editor->GetCurrentAction())
			{
				if (auto* dragAction = action->AsDrag())
					dragAction->m_Clear = true;
				if (auto* selectAction = action->AsSelect())
					selectAction->m_IsActive = false;
			}
		}

		// Draw on top of canvas for subclasses
		OnRenderOnCanvas(editorMin, editorMax);

		ImGui::End(); // Canvas

		// Draw windows of subclasses
		OnRender();
	}

	void NodeGraphEditorBase::EnsureWindowIsDocked(ImGuiWindow* childWindow)
	{
		if (childWindow->DockNode && childWindow->DockNode->ParentNode)
			m_DockIDs[childWindow->ID] = childWindow->DockNode->ParentNode->ID;

		//? For some reason IsAnyMouseDown is not reporting correctly anymore
		//? So for now "auto docking" is disabled
		if (!childWindow->DockIsActive && !ImGui::IsAnyMouseDown() && !childWindow->DockNode && !childWindow->DockNodeIsVisible)
		{
			if (!m_DockIDs[childWindow->ID])
				m_DockIDs[childWindow->ID] = m_MainDockID;

			ImGui::SetWindowDock(childWindow, m_DockIDs[childWindow->ID], ImGuiCond_Always);
		}
	}

	ImColor NodeGraphEditorBase::GetIconColor(int pinTypeID) const
	{
		return GetModel()->GetIconColor(pinTypeID);
	};

	//static ed::PinId s_CurrentComboPinID;

	//=================================================================
	/// NodeEditor Draw Utilities
#pragma region Node Editor Draw Utilities

	void DrawItemActivityOutline(float rounding = 0.0f)
	{
		auto* drawList = ImGui::GetWindowDrawList();
		if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
		{
			drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
				ImColor(60, 60, 60), rounding, 0, 1.5f);
		}
		if (ImGui::IsItemActive())
		{
			drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
				ImColor(50, 50, 50), rounding, 0, 1.0f);
		}
	}

	bool NodeEditorDraw::PropertyBool(choc::value::Value& value)
	{
		bool modified = false;

		bool pinValue = !value.isVoid() ? value.get<bool>() : false;

		if (ImGui::Checkbox("##bool", &pinValue))
		{
			value = choc::value::createBool(pinValue);
			modified = true;
		}

		DrawItemActivityOutline(2.5f);

		return modified;
	}

	bool NodeEditorDraw::PropertyFloat(choc::value::Value& value)
	{
		bool modified = false;

		float pinValue = !value.isVoid() ? value.get<float>() : 0.0f;

		const float valueWidth = ImGui::CalcTextSize(std::to_string(pinValue).c_str()).x + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::PushItemWidth(glm::max(valueWidth, 40.0f));

		if (ImGui::DragFloat("##floatIn", &pinValue, 0.01f, 0.0f, 0.0f, "%g"))
		{
			value = choc::value::createFloat32(pinValue);
			modified = true;
		}

		DrawItemActivityOutline(2.5f);

		ImGui::PopItemWidth();

		return modified;
	}

	bool NodeEditorDraw::PropertyInt(choc::value::Value& value)
	{
		bool modified = false;

		int pinValue = !value.isVoid() ? value.get<int>() : 0;

		const float valueWidth = ImGui::CalcTextSize(std::to_string(pinValue).c_str()).x + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::PushItemWidth(glm::max(valueWidth, 30.0f));

		if (ImGui::DragInt("##int", &pinValue, 0.1f))
		{
			value = choc::value::createInt32(pinValue);
			modified = true;
		}

		DrawItemActivityOutline(2.5f);

		ImGui::PopItemWidth();

		return modified;
	}

	bool NodeEditorDraw::PropertyString(choc::value::Value& value)
	{
		bool modified = false;

		static char buffer[128]{};
		if (!value.isVoid())
		{
			memset(buffer, 0, sizeof(buffer));
			memcpy(buffer, value.get<std::string>().data(), std::min(value.get<std::string>().size(), sizeof(buffer)));
		}

		ImGui::PushItemWidth(100.0f);

		const auto inputTextFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
		ImGui::InputText("##edit", buffer, 127, inputTextFlags);

		DrawItemActivityOutline(2.5f);

		ImGui::PopItemWidth();

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			value = choc::value::createString(std::string(buffer));
			modified = true;
		}

		return modified;
	}

	bool NodeEditorDraw::PropertyObject(choc::value::Value& value, Pin* inputPin, bool& openAssetPopup, NodeEditorModel* model)
	{
		ANT_CORE_VERIFY(inputPin && model);

		bool modified = false;

		// TODO: asset type needs to be retrieved from the model, or from the node

		AssetHandle selected = 0;
		Ref<AudioFile> asset;

		// TODO: position popup below the property button
		//auto canvaslPos = ImGui::GetCursorScreenPos();
		//auto screenPos = ed::ScreenToCanvas(canvaslPos);

		bool assetDropped = false;
		ImGui::SetNextItemWidth(90.0f);

		if (NodeGraphEditorBase::IsAssetHandle(value))
		{
			selected = NodeGraphEditorBase::GetAssetHandleValue(value);
		}

		if (AssetManager::IsAssetHandleValid(selected))
		{
			asset = AssetManager::GetAsset<AudioFile>(selected);
		}
		// TODO: get supported asset type from the pin
		//		Initialize choc::Value class object with correct asset type using helper function
		if (UI::AssetReferenceDropTargetButton("SoundFile", asset, AssetType::SoundGraphSound, assetDropped))
		{
			//ANT_CORE_WARN("Asset clicked");
			openAssetPopup = true;

			s_SelectAssetContext = { inputPin->ID, selected };
		}
		DrawItemActivityOutline(2.5f);

		if (assetDropped)
		{
			if (auto* pin = model->FindPin(s_SelectAssetContext.PinID))
			{
				NodeGraphEditorBase::SetAssetHandleValue(pin->Value, asset->Handle);
				modified = true;
			}
		}

#if 0
		//? This is a bit ugly, but works. Should be changed in the future.
		if (input->Value.isObjectWithClassName("AssetHandle"))
		{
			auto handleStr = input->Value["Value"].get<std::string>();
			selected = std::stoull(handleStr);

			if (AssetManager::IsAssetHandleValid(selected))
			{
				asset = AssetManager::GetAsset<AudioFile>(selected);
			}

			if (asset)
			{
				if (!asset->IsFlagSet(AssetFlag::Missing))
					s_ComboPreview = Project::GetEditorAssetManager()->GetMetadata(asset->Handle).FilePath.stem().string();
				else
					s_ComboPreview = "Missing";
			}
			else
			{
				s_ComboPreview = "Null";
			}
		}

		if (UI::DrawComboPreview(s_ComboPreview.c_str()))
		{
			s_AssetComboSelectedHandle = selected;
			s_AssetComboNode = &node;
			s_AssetComboPin = &input;
			ed::NodeId hoveredNode = ed::GetHoveredNode();
			ImVec2 canvasPosition = ed::GetNodePosition(hoveredNode);
			canvasPosition.y += ed::GetNodeSize(hoveredNode).y;
			s_AssetComboStart = ed::CanvasToScreen(canvasPosition);
			// Uncomment this to have the popup scale with the canvas (I don't recommend it since the font size doesn't scale)
			//s_ComboNodeSize = ed::GetNodeSize(hoveredNode) / ed::GetCurrentZoom();
			s_ShouldAssetComboOpen = true;
		}
#endif
		return modified;
	}

	bool NodeEditorDraw::PropertyEnum(int enumValue, Pin* pin, bool& openEnumPopup, std::function<choc::value::Value(int selected)> constructValue)
	{
		ANT_CORE_VERIFY(pin);

		bool modified = false;

		int pinValue = enumValue;

		std::optional<const std::vector<Token>*> optTokens = pin->EnumTokens;
		if (optTokens.has_value() && optTokens.value() != nullptr)
		{
			ANT_CORE_ASSERT(pinValue >= 0);
			UI::ScopedColour bg(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_FrameBg));

			const auto& tokens = *optTokens.value();

			ImGui::SetNextItemWidth(GetBestWidthForEnumCombo(tokens));
			if (UI::FakeComboBoxButton("##fakeCombo", tokens[pinValue].name.data()))
			{
				openEnumPopup = true;
				s_SelectedEnumContext = { pin->ID, pinValue };
				s_SelectedEnumContext.ConstructValue = constructValue;
			}
		}

		return modified;
	}
#pragma endregion

	//=================================================================

	bool NodeGraphEditorBase::DrawPinPropertyEdit(PinPropertyContext& context)
	{
		bool modified = false;

		Pin* pin = context.pin;

		switch ((EPinType)pin->GetType())
		{
		case EPinType::Bool:	modified = NodeEditorDraw::PropertyBool(pin->Value); break;
		case EPinType::Int:		modified = NodeEditorDraw::PropertyInt(pin->Value); break;
		case EPinType::Float:	modified = NodeEditorDraw::PropertyFloat(pin->Value); break;
		case EPinType::String:	modified = NodeEditorDraw::PropertyString(pin->Value); break;
		case EPinType::Object:	modified = NodeEditorDraw::PropertyObject(pin->Value, pin, context.OpenAssetPopup, context.model); break;
		case EPinType::Enum:	modified = NodeEditorDraw::PropertyEnum(pin->Value.get<int>(), pin, context.OpenEnumPopup,
			[](int selected) { return choc::value::createInt32(selected); }); break;
		default:
			break;
		}

		return modified;
	}

	void NodeGraphEditorBase::DrawNodes()
	{
		utils::BlueprintNodeBuilder builder(UI::GetTextureID(EditorResources::TranslucencyTexture), EditorResources::TranslucencyTexture->GetWidth(), EditorResources::TranslucencyTexture->GetHeight());

		PinPropertyContext dropDownPinContext{ nullptr, GetModel(), false, false };

		//=============================================================
		/// Basic nodes
		for (auto& node : GetModel()->GetNodes())
		{
			if (node->Type != NodeType::Blueprint && node->Type != NodeType::Simple)
				continue;

			const auto isSimple = node->Type == NodeType::Simple;

			bool hasOutputDelegates = false;
			for (auto& output : node->Outputs)
				if (output->IsType(EPinType::DelegatePin))
					hasOutputDelegates = true;

			builder.Begin(ed::NodeId(node->ID));

			/// Draw Header for a non-Simple node
			if (!isSimple)
			{
				builder.Header(node->Color);
				{
					ImGui::Spring(0);
					UI::ScopedColour headerTextColour(ImGuiCol_Text, IM_COL32(210, 210, 210, 255));

					ImGui::TextUnformatted(node->Name.c_str());

					if (m_ShowNodeIDs)
						ImGui::TextUnformatted(("(" + std::to_string(node->ID) + ")").c_str());
					if (m_ShowSortIndices)
						ImGui::TextUnformatted(("(sort index: " + std::to_string(node->SortIndex) + ")").c_str());
				}
				ImGui::Spring(1);

				const float nodeHeaderHeight = 18.0f;
				ImGui::Dummy(ImVec2(0, nodeHeaderHeight));

				// Delegate pin
				if (hasOutputDelegates)
				{
					ImGui::BeginVertical("delegates", ImVec2(0, nodeHeaderHeight));
					ImGui::Spring(1, 0);
					for (auto& output : node->Outputs)
					{
						if (!output->IsType(EPinType::DelegatePin))
							continue;

						auto alpha = ImGui::GetStyle().Alpha;
						if (m_State->NewLinkPin && !GetModel()->CanCreateLink(m_State->NewLinkPin, output) && output != m_State->NewLinkPin)
						{
							alpha = alpha * (48.0f / 255.0f);
						}

						ed::BeginPin(ed::PinId(output->ID), ed::PinKind::Output);
						ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
						ed::PinPivotSize(ImVec2(0, 0));
						ImGui::BeginHorizontal((int)output->ID);
						UI::ScopedStyle alphaStyle(ImGuiStyleVar_Alpha, alpha);
						if (!output->Name.empty())
						{
							ImGui::TextUnformatted(output->Name.c_str());
							ImGui::Spring(0);
						}
						DrawPinIcon(output, GetModel()->IsPinLinked(output->ID), (int)(alpha * 255));
						ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
						ImGui::EndHorizontal();
						ed::EndPin();
					}
					ImGui::Spring(1, 0);
					ImGui::EndVertical();
					ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
				}
				else
				{
					ImGui::Spring(0);
				}

				builder.EndHeader();
			}

			for (auto& input : node->Inputs)
			{
				bool pinValueChanged = false;

				auto alpha = ImGui::GetStyle().Alpha;
				if (m_State->NewLinkPin && !GetModel()->CanCreateLink(m_State->NewLinkPin, input) && input != m_State->NewLinkPin)
				{
					alpha = alpha * (48.0f / 255.0f);
				}

				builder.Input(ed::PinId(input->ID));
				UI::ScopedStyle alphaStyle(ImGuiStyleVar_Alpha, alpha);

				// Pin icon
				DrawPinIcon(input, GetModel()->IsPinLinked(input->ID), (int)(alpha * 255));
				ImGui::Spring(0);

				// Input pin name
				if (!isSimple && !input->Name.empty())
				{
					ImGui::TextUnformatted(input->Name.c_str());
					ImGui::Spring(0);
				}

				// Need to disable editor shortcuts and drag-selecting while editing or dragging input field
				auto deactivateInputIfDragging = [&](bool& wasActive)
				{
					if (ImGui::IsItemActive() && !wasActive)
					{
						m_State->DraggingInputField = true;
						ed::EnableShortcuts(false);
						wasActive = true;
					}
					else if (!ImGui::IsItemActive() && wasActive)
					{
						m_State->DraggingInputField = false;
						ed::EnableShortcuts(true);
						wasActive = false;
					}
				};

				// Draw input fields for not linked non-array pins
				// TODO: implementation might want to draw property edit for linked pins as well(?)
				if (input->Storage != StorageKind::Array && !GetModel()->IsPinLinked(input->ID))
				{
					UI::ScopedStyle frameRounding(ImGuiStyleVar_FrameRounding, 2.5f);

					// TODO: handle implementation specific types
					PinPropertyContext context{ input, GetModel(), false, false };
					pinValueChanged = DrawPinPropertyEdit(context);

					if (context.OpenAssetPopup || context.OpenEnumPopup)
						dropDownPinContext = context;

					static bool wasActive = false;
					deactivateInputIfDragging(wasActive); //? this might not work, if it requires unique bool for each property type, or if property edit was not even drawn

					ImGui::Spring(0);
				}

				if (pinValueChanged && GetModel()->onPinValueChanged)
				{
					GetModel()->onPinValueChanged(node->ID, input->ID);
				}

				builder.EndInput();
			}

			// Setting colour of the Message Node input field to dark
			UI::ScopedColour frameBgColour(ImGuiCol_FrameBg, ImVec4{ 0.08f, 0.08f, 0.08f, 1.0f });

			// Large icon for Simple nodes
			if (isSimple)
			{
				builder.Middle();

				ImGui::Spring(1, 0);
				UI::ScopedColour textColour(ImGuiCol_Text, IM_COL32(210, 210, 210, 255));

				// TODO: handle simple nodes display icon properly
				if (const char* displayIcon = GetIconForSimpleNode(node->Name))
				{
					UI::ScopedColour textColour(ImGuiCol_Text, Colours::Theme::text);
					UI::ScopedFont largeFont(ImGui::GetIO().Fonts->Fonts[1]);
					ImGui::TextUnformatted(displayIcon);
				}
				else
					ImGui::TextUnformatted(node->Name.c_str());

				if (m_ShowNodeIDs)
					ImGui::TextUnformatted(("(" + std::to_string(node->ID) + ")").c_str());

				ImGui::Spring(1, 0);
			}

			for (auto& output : node->Outputs)
			{
				if (!isSimple && output->IsType(EPinType::DelegatePin))
					continue;

				auto alpha = ImGui::GetStyle().Alpha;
				if (m_State->NewLinkPin && !GetModel()->CanCreateLink(m_State->NewLinkPin, output) && output != m_State->NewLinkPin)
				{
					alpha = alpha * (48.0f / 255.0f);
				}

				UI::ScopedStyle alphaStyleOverride(ImGuiStyleVar_Alpha, alpha);

				builder.Output(ed::PinId(output->ID));

				// Draw output pin name
				if (!isSimple && !output->Name.empty() && output->Name != "Message")
				{
					ImGui::Spring(0);
					ImGui::TextUnformatted(output->Name.c_str());
				}
				ImGui::Spring(0);
				DrawPinIcon(output, GetModel()->IsPinLinked(output->ID), (int)(alpha * 255));
				builder.EndOutput();

				// TODO: implementation might want to draw property edit for outputs as well
			}

			builder.End();

			int radius = EditorResources::ShadowTexture->GetHeight();// *1.4;
			UI::DrawShadow(EditorResources::ShadowTexture, radius);
		}

		//=============================================================
		/// Comment nodes
		for (auto& node : GetModel()->GetNodes())
		{
			if (node->Type != NodeType::Comment)
				continue;

			//! This demonstrates low-level custom node drawing

			const float commentAlpha = 0.75f;

			// Need to disable editor shortcuts and drag-selecting while editing or dragging input field
			auto deactivateInputIfDragging = [&](bool& wasActive)
			{
				if (ImGui::IsItemActive() && !wasActive)
				{
					m_State->DraggingInputField = true;
					ed::EnableShortcuts(false);
					wasActive = true;
				}
				else if (!ImGui::IsItemActive() && wasActive)
				{
					m_State->DraggingInputField = false;
					ed::EnableShortcuts(true);
					wasActive = false;
				}
			};

			//ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 20));
			ed::PushStyleColor(ed::StyleColor_NodeBg, node->Color);
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 24));
			ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32_DISABLE);
			ed::BeginNode(ed::NodeId(node->ID));
			{
				UI::ScopedStyle nodeStyle(ImGuiStyleVar_Alpha, commentAlpha);

				UI::ScopedID nodeID((int)node->ID);
				ImGui::BeginVertical("content");
				ImGui::BeginHorizontal("horizontal");
				{
					UI::ShiftCursorX(6.0f);
					UI::ScopedColour frameColour(ImGuiCol_FrameBg, IM_COL32(255, 255, 255, 0));
					UI::ScopedColour textColour(ImGuiCol_Text, IM_COL32(220, 220, 220, 255));
					UI::ScopedFont largeFont(ImGui::GetIO().Fonts->Fonts[1]);

					static bool wasActive = false;

					static char buffer[256]{};
					memset(buffer, 0, sizeof(buffer));
					memcpy(buffer, node->Name.data(), std::min(node->Name.size(), sizeof(buffer)));

					const auto inputTextFlags = ImGuiInputTextFlags_AutoSelectAll;

					// Update our node size if backend node size has been changed
					const ImVec2 nodeSize = ed::GetNodeSize((uint64_t)node->ID);
					float availableWidth = 0.0f;
					if (nodeSize == ImVec2(0.0f, 0.0f))
					{
						availableWidth = node->Size.x;
					}
					else
					{
						availableWidth = nodeSize.x;

						if (nodeSize != node->Size)
							node->Size = nodeSize;
					}

					ImVec2 textSize = ImGui::CalcTextSize((node->Name + "00").c_str());
					textSize.x = std::min(textSize.x, availableWidth - 16.0f);

					ImGui::PushItemWidth(textSize.x);
					//ImGui::PushTextWrapPos(availableWidth); //? doesn't work
					{
						UI::ScopedStyle alpha(ImGuiStyleVar_Alpha, 1.0f);

						// Draw text shadow
						{
							UI::ScopedColour textShadow(ImGuiCol_Text, IM_COL32_BLACK);
							const auto pos = ImGui::GetCursorPos();
							UI::ShiftCursor(4.0f, 4.0f);
							ImGui::TextUnformatted(node->Name.c_str());
							ImGui::SetCursorPos(pos);
						}

						if (ImGui::InputText("##edit", buffer, 255, inputTextFlags))
						{
							node->Name = buffer;
						}
					}
					//ImGui::PopTextWrapPos();
					ImGui::PopItemWidth();

					deactivateInputIfDragging(wasActive);
				}
				ImGui::EndHorizontal();
				ed::Group(node->Size);
				ImGui::EndVertical();
			}
			ed::EndNode();
			ImGui::PopStyleColor(); // Border
			ed::PopStyleColor(2); // StyleColor_NodeBg, StyleColor_NodeBorder

			// Pupup hint when zoomed out
			if (ed::BeginGroupHint(ed::NodeId(node->ID)))
			{
				auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

				auto min = ed::GetGroupMin();

				ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
				ImGui::BeginGroup();
				ImGui::TextUnformatted(node->Name.c_str());
				ImGui::EndGroup();

				auto drawList = ed::GetHintBackgroundDrawList();

				auto hintBounds = UI::GetItemRect();
				auto hintFrameBounds = UI::RectExpanded(hintBounds, 8, 4);

				drawList->AddRectFilled(
					hintFrameBounds.GetTL(),
					hintFrameBounds.GetBR(),
					IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 1.0f);

				drawList->AddRect(
					hintFrameBounds.GetTL(),
					hintFrameBounds.GetBR(),
					IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 1.0f);
			}
			ed::EndGroupHint();
		}


		//=============================================================
		/// Deferred combo boxes
		ed::Suspend();
		{
			/// Asset pin search popup
			{
				if (dropDownPinContext.OpenAssetPopup)
					ImGui::OpenPopup("AssetSearch");

				// TODO: get itme rect translated from canvas to screen coords and positin popup properly
				Ref<AudioFile> asset;
				//static AssetHandle selected;

				// TODO: AssetSearchList should be combined with the button that opens it up.
				//		At the moment they are separate.
				bool clear = false;
				if (UI::Widgets::AssetSearchPopup("AssetSearch", asset->GetStaticType(), s_SelectAssetContext.SelectedAssetHandle, &clear, "Search Asset", ImVec2{ 250.0f, 300.0f }))
				{
					if (auto* pin = GetModel()->FindPin(s_SelectAssetContext.PinID))
					{
						SetAssetHandleValue(pin->Value, clear ? 0 : s_SelectAssetContext.SelectedAssetHandle);

						if (GetModel()->onPinValueChanged)
						{
							GetModel()->onPinValueChanged(pin->NodeID, pin->ID);
						}
					}

					s_SelectAssetContext.PinID = 0;
					s_SelectAssetContext.SelectedAssetHandle = 0;
				}
			}

			/// Enum combo box
			{
				if (dropDownPinContext.OpenEnumPopup && GetModel()->FindPin(s_SelectedEnumContext.PinID))
					ImGui::OpenPopup("##EnumPopup");

				if (auto* pin = GetModel()->FindPin(s_SelectedEnumContext.PinID))
				{
					bool changed = false;
					ANT_CORE_ASSERT(pin->EnumTokens.has_value() && pin->EnumTokens.value() != nullptr);
					const auto& tokens = **pin->EnumTokens;

					ImGui::SetNextWindowSize({ GetBestWidthForEnumCombo(tokens), 0.0f });
					if (UI::BeginPopup("##EnumPopup", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
					{
						for (int i = 0; i < tokens.size(); ++i)
						{
							ImGui::PushID(i);
							const bool itemSelected = (i == s_SelectedEnumContext.SelectedValue);
							if (ImGui::Selectable(tokens[i].name.data(), itemSelected))
							{
								changed = true;
								s_SelectedEnumContext.SelectedValue = i;
							}
							if (itemSelected)
								ImGui::SetItemDefaultFocus();
							ImGui::PopID();
						}
						UI::EndPopup();
					}

					if (changed)
					{
						pin->Value = s_SelectedEnumContext.ConstructValue(s_SelectedEnumContext.SelectedValue);
						if (GetModel()->onPinValueChanged)
							GetModel()->onPinValueChanged(pin->NodeID, pin->ID);
					}
				}
			}
		}
		ed::Resume();
	}

	void NodeGraphEditorBase::DrawPinIcon(const Pin* pin, bool connected, int alpha)
	{
		const int pinIconSize = GetPinIconSize();

		//IconType iconType;
		ImColor  color = GetIconColor(pin->GetType());
		color.Value.w = alpha / 255.0f;

		// Draw a highlight if hovering over this pin or its label
		if (ed::PinId(pin->ID) == ed::GetHoveredPin())
		{
			auto* drawList = ImGui::GetWindowDrawList();
			auto size = ImVec2(static_cast<float>(pinIconSize), static_cast<float>(pinIconSize));
			auto cursorPos = ImGui::GetCursorScreenPos();
			const auto outline_scale = size.x / 24.0f;
			const auto extra_segments = static_cast<int>(2 * outline_scale); // for full circle
			const auto radius = 0.5f * size.x / 2.0f - 0.5f;
			const auto centre = ImVec2(cursorPos.x + size.x / 2.0f, cursorPos.y + size.y / 2.0f);
			drawList->AddCircleFilled(centre, 0.5f * size.x, IM_COL32(255, 255, 255, 30), 12 + extra_segments);
		}

		// If this pin accepting a link, draw it as connected
		bool acceptingLink = IsPinAcceptingLink(pin);


		if (pin->Storage == StorageKind::Array)
			ax::Widgets::IconGrid(ImVec2(static_cast<float>(pinIconSize), static_cast<float>(pinIconSize)), connected || acceptingLink, color, ImColor(32, 32, 32, alpha));
		else
		{
			Ref<Texture2D> image;

			if (pin->IsType(EPinType::Flow))
			{
				image = connected || acceptingLink ? EditorResources::PinFlowConnectIcon : EditorResources::PinFlowDisconnectIcon;
			}
			else
			{
				image = connected || acceptingLink ? EditorResources::PinValueConnectIcon : EditorResources::PinValueDisconnectIcon;
			}

			const auto iconWidth = image->GetWidth();
			const auto iconHeight = image->GetHeight();
			ax::Widgets::ImageIcon(ImVec2(static_cast<float>(iconWidth), static_cast<float>(iconHeight)), UI::GetTextureID(image), connected, (float)pinIconSize, color, ImColor(32, 32, 32, alpha));
		}
	}

} // namespace Ant