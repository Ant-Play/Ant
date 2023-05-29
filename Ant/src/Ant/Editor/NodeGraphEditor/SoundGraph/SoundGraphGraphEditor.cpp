#include "antpch.h"
#include "SoundGraphGraphEditor.h"
#include "SoundGraphEditorTypes.h"
#include "SoundGraphNodeEditorModel.h"

#include "Ant/Asset/AssetManager.h"
#include "Ant/Editor/NodeGraphEditor/NodeGraphEditorContext.h"

#include "choc/text/choc_StringUtilities.h"
#include "Ant/Vendor/imgui-node-editor/widgets.h"
#include "imgui-node-editor/builders.h"
#include "imgui-node-editor/imgui_node_editor.h"

namespace ed = ax::NodeEditor;
namespace utils = ax::NodeEditor::Utilities;

namespace Ant
{
	struct SoundGraphNodeGraphEditor::SelectedItem
	{
		enum EType
		{
			Invalid = 0, Input, Output, LocalVariable, Node
		};
		EType Type{ Invalid };

		std::string Name;
		UUID NodeID = 0;

		operator bool() const
		{
			return (!Name.empty() || NodeID) && Type != Invalid;
		}

		bool IsSelected(ESoundGraphPropertyType type, std::string_view name) const { return Type == FromPropertyType(type) && Name == name; }
		bool IsSelected(EType type, std::string_view name) const { return Type == type && Name == name; }

		void Select(ESoundGraphPropertyType type, std::string_view name) { Type = FromPropertyType(type); Name = name; NodeID = 0; ed::ClearSelection(); }
		/** Select property type */
		void Select(EType type, std::string_view name) { Type = type; Name = name; NodeID = 0; ed::ClearSelection(); }
		/** Select node */
		void Select(EType type, UUID nodeID) { Type = type; NodeID = nodeID; Name.clear(); }

		void Clear() { Type = Invalid; Name.clear(); NodeID = 0; }

		ESoundGraphPropertyType GetPropertyType() const
		{
			switch (Type)
			{
			case SelectedItem::Input: return ESoundGraphPropertyType::Input;
			case SelectedItem::Output: return ESoundGraphPropertyType::Output;
			case SelectedItem::LocalVariable: return ESoundGraphPropertyType::LocalVariable;
			case SelectedItem::Node:
			case SelectedItem::Invalid:
			default: return ESoundGraphPropertyType::Invalid;
			}
		}

		EType FromPropertyType(ESoundGraphPropertyType type) const
		{
			switch (type)
			{
			case ESoundGraphPropertyType::Input: return SelectedItem::Input;
			case ESoundGraphPropertyType::Output: return SelectedItem::Output;
			case ESoundGraphPropertyType::LocalVariable: return SelectedItem::LocalVariable;
			case ESoundGraphPropertyType::Invalid:
			default: return SelectedItem::Invalid;
			}
		}
	};

	//=============================================================================================
	/// SoundGraph Node Graph Editor
	SoundGraphNodeGraphEditor::SoundGraphNodeGraphEditor() : NodeGraphEditorBase("SoundGraph Sound")
	{
		m_SelectedItem = CreateScope<SelectedItem>();

		onNodeListPopup = [&](bool searching, std::string_view searchedString)
		{
			Node* newNode = nullptr;

			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(140, 140, 140, 255));
			if (UI::ContextMenuHeader("Graph Inputs", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PopStyleColor(); // header Text

				ImGui::Indent();
				//-------------------------------------------------
				if (searching)
				{
					for (const auto& graphInput : m_Model->GetInputs().GetNames())
					{
						if (UI::IsMatchingSearch("Graph Inputs", searchedString)
							|| UI::IsMatchingSearch(graphInput, searchedString))
						{
							if (ImGui::MenuItem(choc::text::replace(graphInput, "_", " ").c_str()))		// TODO: SpawnGraphOutputNode
								newNode = m_Model->SpawnGraphInputNode(graphInput);
						}
					}
				}
				else
				{
					for (const auto& graphInput : m_Model->GetInputs().GetNames())
					{
						if (ImGui::MenuItem(choc::text::replace(graphInput, "_", " ").c_str()))			// TODO: SpawnGraphOutputNode
							newNode = m_Model->SpawnGraphInputNode(graphInput);
					}
				}

				ImGui::Unindent();
			}
			else
			{
				ImGui::PopStyleColor(); // header Text
			}

			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(140, 140, 140, 255));
			if (UI::ContextMenuHeader("Local Variables", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PopStyleColor(); // header Text

				ImGui::Indent();
				//-------------------------------------------------
				if (searching)
				{
					for (const auto& localVariable : m_Model->GetLocalVariables().GetNames())
					{
						const std::string setter = localVariable + " Set";
						const std::string getter = localVariable + " Get";

						if (UI::IsMatchingSearch("Local Variables", searchedString)
							|| UI::IsMatchingSearch(setter, searchedString)
							|| UI::IsMatchingSearch(getter, searchedString))
						{
							if (ImGui::MenuItem(choc::text::replace(setter, "_", " ").c_str()))
								newNode = m_Model->SpawnLocalVariableNode(localVariable, false);

							if (ImGui::MenuItem(choc::text::replace(getter, "_", " ").c_str()))
								newNode = m_Model->SpawnLocalVariableNode(localVariable, true);
						}
					}
				}
				else
				{
					for (const auto& localVariable : m_Model->GetLocalVariables().GetNames())
					{
						const std::string setter = localVariable + " Set";
						const std::string getter = localVariable + " Get";

						if (ImGui::MenuItem(choc::text::replace(setter, "_", " ").c_str()))
							newNode = m_Model->SpawnLocalVariableNode(localVariable, false);

						if (ImGui::MenuItem(choc::text::replace(getter, "_", " ").c_str()))
							newNode = m_Model->SpawnLocalVariableNode(localVariable, true);
					}
				}

				ImGui::Unindent();
			}
			else
			{
				ImGui::PopStyleColor(); // header Text
			}

			return newNode;
		};
	};
	SoundGraphNodeGraphEditor::~SoundGraphNodeGraphEditor() = default;

	ImGuiWindowFlags SoundGraphNodeGraphEditor::GetWindowFlags()
	{
		return ImGuiWindowFlags_NoCollapse;
	}

	void SoundGraphNodeGraphEditor::OnWindowStylePush()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

		/*if (ImGui::Begin("Style Editor"))
			ImGui::ShowStyleEditor();

		ImGui::End();*/
	}

	void SoundGraphNodeGraphEditor::OnWindowStylePop()
	{
		ImGui::PopStyleVar(2); // ImGuiStyleVar_WindowPadding, ImGuiStyleVar_FrameBorderSize
	}

	NodeEditorModel* SoundGraphNodeGraphEditor::GetModel()
	{
		return m_Model.get();
	}

	const NodeEditorModel* SoundGraphNodeGraphEditor::GetModel() const
	{
		return m_Model.get();
	}

	void SoundGraphNodeGraphEditor::SetAsset(const Ref<Asset>& asset)
	{
		m_Model = std::make_unique<SoundGraphNodeEditorModel>(asset.As<SoundGraphAsset>());
		NodeGraphEditorBase::SetAsset(asset);

		const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(asset->Handle);
		SetTitle(metadata.FilePath.stem().string());
	}

	const char* SoundGraphNodeGraphEditor::GetIconForSimpleNode(const std::string& simpleNodeIdentifier) const
	{
		if (choc::text::contains(simpleNodeIdentifier, "Add")) return "+";
		if (choc::text::contains(simpleNodeIdentifier, "Subtract")) return "-";
		if (choc::text::contains(simpleNodeIdentifier, "Mult")) return "x";
		if (choc::text::contains(simpleNodeIdentifier, "Divide")) return "/";
		if (choc::text::contains(simpleNodeIdentifier, "Modulo")) return "%";
		if (choc::text::contains(simpleNodeIdentifier, "Pow")) return "^";
		if (choc::text::contains(simpleNodeIdentifier, "Log")) return "log";
		if (choc::text::contains(simpleNodeIdentifier, "Less")) return "<";

		else return nullptr;
	}

	void SoundGraphNodeGraphEditor::DrawPinIcon(const Pin* pin, bool connected, int alpha)
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

			if (pin->IsType(Nodes::SGTypes::ESGPinType::Audio))
			{
				image = connected || acceptingLink ? EditorResources::PinAudioConnectIcon : EditorResources::PinAudioDisconnectIcon;
			}
			/*else if (pin->IsType(Nodes::SGTypes::ESGPinType::Flow))
			{
				image = connected || acceptingLink ? EditorResources::PinFlowConnectIcon : EditorResources::PinFlowDisconnectIcon;
			}*/
			else
			{
				image = connected || acceptingLink ? EditorResources::PinValueConnectIcon : EditorResources::PinValueDisconnectIcon;
			}

			const auto iconWidth = image->GetWidth();
			const auto iconHeight = image->GetHeight();
			ax::Widgets::ImageIcon(ImVec2(static_cast<float>(iconWidth), static_cast<float>(iconHeight)), UI::GetTextureID(image), connected, (float)pinIconSize, color, ImColor(32, 32, 32, alpha));
		}
	}

	void SoundGraphNodeGraphEditor::OnUpdate(Timestep ts)
	{
		if (IsOpen())
			m_Model->OnUpdate(ts);
	}

	void SoundGraphNodeGraphEditor::OnRender()
	{
		ImGui::SetNextWindowClass(GetWindowClass());

		if (ImGui::Begin("Toolbar##SoundGraphGraphEditor", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			//EnsureWindowIsDocked(ImGui::GetCurrentWindow());
			DrawToolbar();
		}
		ImGui::End(); // Toolbar

		ImGui::SetNextWindowClass(GetWindowClass());
		if (ImGui::Begin("Graph Details", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse))
		{
			//EnsureWindowIsDocked(ImGui::GetCurrentWindow());
			DrawDetailsPanel();
		}
		ImGui::End(); // VariablesPanel

		ImGui::SetNextWindowClass(GetWindowClass());

		if (ImGui::Begin("Graph Inputs##SoundGraphGraphEditor", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollWithMouse))
		{
			//EnsureWindowIsDocked(ImGui::GetCurrentWindow());
			DrawGraphIO();
		}
		ImGui::End(); // VariablesPanel
	}

	void SoundGraphNodeGraphEditor::OnRenderOnCanvas(ImVec2 min, ImVec2 max)
	{
		uint64_t currentMs = m_Model->GetCurrentPlaybackFrame() / 48;
		// Not an ideal check, but there's probably no other case where current
		// playback position is 0 except for when the player isn't playing.
		if (currentMs > 0)
		{
			UI::ScopedFont largeFont(ImGui::GetIO().Fonts->Fonts[1]);

			const ImVec2 timeCodePosition({ min.x + 20.0f, min.y + 10.0f });
			const std::string timecodeText = Utils::DurationToString(std::chrono::milliseconds(currentMs));

			ImGui::GetWindowDrawList()->AddText(timeCodePosition, IM_COL32(255, 255, 255, 60), timecodeText.c_str());
		}
	}

	bool SoundGraphNodeGraphEditor::DrawPinPropertyEdit(PinPropertyContext& context)
	{
		bool modified = false;

		Pin* pin = context.pin;

		switch ((Nodes::SGTypes::ESGPinType)pin->GetType())
		{
		case Nodes::SGTypes::ESGPinType::Bool:		modified = NodeEditorDraw::PropertyBool(pin->Value); break;
		case Nodes::SGTypes::ESGPinType::Int:		modified = NodeEditorDraw::PropertyInt(pin->Value); break;
		case Nodes::SGTypes::ESGPinType::Float:		modified = NodeEditorDraw::PropertyFloat(pin->Value); break;
		case Nodes::SGTypes::ESGPinType::String:	modified = NodeEditorDraw::PropertyString(pin->Value); break;
		case Nodes::SGTypes::ESGPinType::Object:	modified = NodeEditorDraw::PropertyObject(pin->Value, pin, context.OpenAssetPopup, context.model); break;
		case Nodes::SGTypes::ESGPinType::Enum:
		{
			const int enumValue = pin->Value["Value"].get<int>();

			// TODO: JP. not ideal to do this callback style value assignment, should make it more generic.
			auto constructEnumValue = [](int selected) { return ValueFrom(Nodes::SGTypes::TSGEnum{ selected }); };

			modified = NodeEditorDraw::PropertyEnum(enumValue, pin, context.OpenEnumPopup, constructEnumValue);
		}
		break;
		default:
			break;
		}

		return modified;
	}

	void SoundGraphNodeGraphEditor::OnClose()
	{
		if (m_Model->onStop)
			m_Model->onStop(true);

		NodeGraphEditorBase::OnClose();
	}

	void SoundGraphNodeGraphEditor::DrawToolbar()
	{
		auto* drawList = ImGui::GetWindowDrawList();

		const float spacing = 16.0f;

		ImGui::BeginHorizontal("ToolbarHorizontalLayout", ImGui::GetContentRegionAvail());
		ImGui::Spring(0.0f, spacing);

		// Compile Button
		{
			float compileIconWidth = EditorResources::CompileIcon->GetWidth() * 0.9f;
			float compileIconHeight = EditorResources::CompileIcon->GetWidth() * 0.9f;
			if (ImGui::InvisibleButton("compile", ImVec2(compileIconWidth, compileIconHeight)))
			{
				m_Model->CompileGraph();
			}
			UI::DrawButtonImage(EditorResources::CompileIcon, IM_COL32(235, 165, 36, 200),
				IM_COL32(235, 165, 36, 255),
				IM_COL32(235, 165, 36, 120));

			UI::SetTooltip("Compile");

			if (m_Model->IsPlayerDirty())
			{
				UI::ShiftCursorX(-6.0f);
				ImGui::Text("*");
			}
		}

		// Save Button
		{
			const int saveIconWidth = EditorResources::SaveIcon->GetWidth();
			const int saveIconHeight = EditorResources::SaveIcon->GetWidth();

			if (ImGui::InvisibleButton("saveGraph", ImVec2((float)saveIconWidth, (float)saveIconHeight)))
			{
				m_Model->SaveAll();
			}
			const int iconOffset = 4;
			auto iconRect = UI::GetItemRect();
			iconRect.Min.y += iconOffset;
			iconRect.Max.y += iconOffset;

			UI::DrawButtonImage(EditorResources::SaveIcon, IM_COL32(102, 204, 163, 200),
				IM_COL32(102, 204, 163, 255),
				IM_COL32(102, 204, 163, 120), iconRect);

			UI::SetTooltip("Save graph");
		}

		// Viewport to content
		{
			if (ImGui::Button("Navigate to Content"))
				ed::NavigateToContent();
		}

		const float leftSizeOffset = ImGui::GetCursorPosX();

		ImGui::Spring();

		// Playback butons
		// ---------------

		// Play Button
		{
			if (ImGui::InvisibleButton("PlayGraphButton", ImVec2(ImGui::GetTextLineHeightWithSpacing() + 4.0f,
				ImGui::GetTextLineHeightWithSpacing() + 4.0f)))
			{
				if (m_Model->IsPlayerDirty())
					m_Model->CompileGraph();

				if (!m_Model->IsPlayerDirty() && m_Model->onPlay)
					m_Model->onPlay();
			}

			UI::DrawButtonImage(EditorResources::PlayIcon, IM_COL32(102, 204, 163, 200),
				IM_COL32(102, 204, 163, 255),
				IM_COL32(102, 204, 163, 120),
				UI::RectExpanded(UI::GetItemRect(), 1.0f, 1.0f));
		}

		// Stop Button
		{
			if (ImGui::InvisibleButton("StopGraphButton", ImVec2(ImGui::GetTextLineHeightWithSpacing() + 4.0f,
				ImGui::GetTextLineHeightWithSpacing() + 4.0f)))
			{
				if (m_Model->onStop) m_Model->onStop(false);
			}

			UI::DrawButtonImage(EditorResources::StopIcon, IM_COL32(102, 204, 163, 200),
				IM_COL32(102, 204, 163, 255),
				IM_COL32(102, 204, 163, 120),
				UI::RectExpanded(UI::GetItemRect(), -1.0f, -1.0f));
		}

		ImGui::Spring(1.0f, leftSizeOffset);

		ImGui::Spring(0.0f, spacing);
		ImGui::EndHorizontal();
	}

	void SoundGraphNodeGraphEditor::DrawDetailsPanel()
	{
		auto& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		std::vector<ed::NodeId> selectedNodes;
		std::vector<ed::LinkId> selectedLinks;
		selectedNodes.resize(ed::GetSelectedObjectCount());
		selectedLinks.resize(ed::GetSelectedObjectCount());

		int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), static_cast<int>(selectedNodes.size()));
		int linkCount = ed::GetSelectedLinks(selectedLinks.data(), static_cast<int>(selectedLinks.size()));

		selectedNodes.resize(nodeCount);
		selectedLinks.resize(linkCount);

		UI::ShiftCursorY(12.0f);
		{
			ImGui::CollapsingHeader("##NODES", ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf);
			ImGui::SetItemAllowOverlap();
			ImGui::SameLine();
			UI::ShiftCursorX(-8.0f);
			ImGui::Text("NODES");
		}

		if (ImGui::BeginListBox("##nodesListBox", ImVec2(FLT_MIN, 0)))
		{
			for (auto& node : m_Model->GetNodes())
			{
				UI::ScopedID nodeID((int)node->ID);
				auto start = ImGui::GetCursorScreenPos();

				bool isSelected = std::find(selectedNodes.begin(), selectedNodes.end(), ed::NodeId(node->ID)) != selectedNodes.end();
				if (ImGui::Selectable((node->Name + "##" + std::to_string(node->ID)).c_str(), &isSelected))
				{
					if (io.KeyCtrl)
					{
						if (isSelected)
							ed::SelectNode(ed::NodeId(node->ID), true);
						else
							ed::DeselectNode(ed::NodeId(node->ID));
					}
					else
						ed::SelectNode(ed::NodeId(node->ID), false);

					ed::NavigateToSelection();
				}
				if (UI::IsItemHovered() && !node->State.empty())
					ImGui::SetTooltip("State: %s", node->State.c_str());

				//? Display node IDs
				/*auto id = std::string("(") + std::to_string(node.ID) + ")";
				auto textSize = ImGui::CalcTextSize(id.c_str(), nullptr);

				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - textSize.x);
				ImGui::Text(id.c_str());*/
			}

			ImGui::EndListBox();
		}
	}

	void SoundGraphNodeGraphEditor::DrawNodeDetails(UUID nodeID)
	{
		auto* node = m_Model->FindNode(nodeID);
		if (!node)
			return;

		auto propertyType = m_Model->GetPropertyTypeOfNode(node);
		if (propertyType != ESoundGraphPropertyType::Invalid)
		{
			std::string_view propertyName;
			switch (propertyType)
			{
			case Ant::Input: propertyName = node->Outputs[0]->Name; break;
			case Ant::Output: propertyName = node->Inputs[0]->Name; break;
			case Ant::LocalVariable:
			default: propertyName = node->Name; break;
			}
			DrawPropertyDetails(propertyType, propertyName);
			return;
		}

		//! For now we only draw details for a Comment node
		if (node->Type != NodeType::Comment)
			return;

		if (UI::PropertyGridHeader("Comment"))
		{
			UI::BeginPropertyGrid();

			// Comment colour
			{
				glm::vec4 colour;
				memcpy(&colour.x, &node->Color.Value.x, sizeof(float) * 4);

				if (UI::PropertyColor("Colour", colour))
					memcpy(&node->Color.Value.x, &colour.x, sizeof(float) * 4);
			}

			UI::EndPropertyGrid();
			ImGui::TreePop();
		}
	}

	void SoundGraphNodeGraphEditor::DrawPropertyDetails(uint8_t soundGraphPropertyType, std::string_view propertyName)
	{
		ESoundGraphPropertyType propertyType = (ESoundGraphPropertyType)soundGraphPropertyType;

		const float panelWidth = ImGui::GetContentRegionAvail().x;

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Indent();

		std::string oldName(propertyName);
		std::string newName(propertyName);

		choc::value::Value value = m_Model->GetPropertySet(propertyType).GetValue(newName);

		// Name
		{
			static char buffer[128]{};
			memset(buffer, 0, sizeof(buffer));
			memcpy(buffer, newName.data(), std::min(newName.size(), sizeof(buffer)));

			const auto inputTextFlags = ImGuiInputTextFlags_AutoSelectAll
				| ImGuiInputTextFlags_EnterReturnsTrue
				| ImGuiInputTextFlags_CallbackAlways;

			ImGui::Text("Input Name");
			ImGui::SameLine();
			UI::ShiftCursorY(-3.0f);

			// TODO: move this into some sort of utility header
			auto validateName = [](ImGuiInputTextCallbackData* data) -> int
			{
				const auto isDigit = [](char c) { return c >= '0' && c <= '9'; };
				const auto isSafeIdentifierChar = [&isDigit](char c)
				{
					return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == ' ' || isDigit(c);
				};

				const int lastCharPos = glm::max(data->CursorPos - 1, 0);
				char lastChar = data->Buf[lastCharPos];

				if (data->BufTextLen >= 1)
				{
					// Must not start with '_' or ' '
					if (data->Buf[0] == '_' || data->Buf[0] == ' ' || isDigit(data->Buf[0]) || !isSafeIdentifierChar(data->Buf[0]))
						data->DeleteChars(0, 1);
				}

				if (data->BufTextLen < 3)
					return 0;

				if (lastChar == ' ')
				{
					// Block double whitespaces
					std::string_view str(data->Buf);
					size_t doubleSpacePos = str.find("  ");
					if (doubleSpacePos != std::string::npos)
						data->DeleteChars((int)doubleSpacePos, 1);

					return 1;
				}
				else if (!isSafeIdentifierChar(lastChar))
				{
					data->DeleteChars(lastCharPos, 1);
				}

				return 0;
			};

			ImGui::InputText("##inputName", buffer, 127, inputTextFlags, validateName);

			UI::DrawItemActivityOutline(2.5f, true, Colours::Theme::accent);


			if (ImGui::IsItemDeactivatedAfterEdit() && buffer[0])
			{
				if (m_Model->IsGraphPropertyNameValid(propertyType, buffer))
				{
					// Rename item in the list
					newName = buffer;
					newName = choc::text::trim(newName);
					m_Model->RenameProperty(propertyType, oldName, newName);

					m_SelectedItem->Name = buffer;
				}
				else
				{
					// TODO: show some informative warning
				}
			}
		}

		// Type & Value
		{
			std::string typeName;
			int32_t selected = 0;

			static std::vector<std::string> types{ "Float", "Int", "Bool", "WaveAsset", "Trigger" }; // TODO: Function/Event/Trigger

			choc::value::Type type = value.getType();
			choc::value::Type arrayType;

			bool isArrayOrVector = false;

			if (value.isArray())
			{
				arrayType = type;
				isArrayOrVector = true;
				type = type.getElementType();
			}

			if (type.isPrimitive() || type.isString() || type.isObject())
			{
				if (type.isFloat())   selected = 0;
				else if (type.isInt32())   selected = 1;
				else if (type.isBool())    selected = 2;
				else if (IsAssetHandle(type))
					selected = 3;
				else ANT_CORE_ERROR("Invalid type of Graph Input!");
			}
			else
			{
				if (type.isVoid()) selected = 4; // Trigger
				else
					ANT_CORE_ERROR("Invalid type of Graph Input!");
			}

			auto createValueFromSelectedIndex = [](int32_t index)
			{
				if (index == 0) return choc::value::Value(0.0f);
				if (index == 1) return choc::value::Value(int32_t(0));
				if (index == 2) return choc::value::Value(false);
				if (index == 3) return ValueFrom(UUID(0));
				if (index == 4) return choc::value::Value(); // Trigger
				// TODO: maybe make custom Value type object to represent triggers?

				return choc::value::Value();
			};


			// Change the type the value, or the type of array element
			ImGui::SetCursorPosX(0.0f);

			const float checkboxWidth = 84.0f;
			ImGui::BeginChildFrame(ImGui::GetID("TypeRegion"), ImVec2(ImGui::GetContentRegionAvail().x - checkboxWidth, ImGui::GetFrameHeight() * 1.4f), ImGuiWindowFlags_NoBackground);
			ImGui::Columns(2);

			auto getColourForType = [&](int optionNumber)
			{
				return GetTypeColour(createValueFromSelectedIndex(optionNumber));
			};

			if (UI::PropertyDropdown("Type", types, (int32_t)types.size(), &selected, getColourForType))
			{
				choc::value::Value newValue = createValueFromSelectedIndex(selected);

				if (isArrayOrVector && selected != 4) // NOTE (Tim) : Checking that the selected is not a trigger.
				{
					choc::value::Value arrayValue = choc::value::createArray(value.size(), [&](uint32_t i) { return choc::value::Value(newValue.getType()); });

					m_Model->ChangePropertyType(propertyType, newName, arrayValue);
				}
				else
				{
					m_Model->ChangePropertyType(propertyType, newName, newValue);
				}
			}
			UI::DrawItemActivityOutline(2.5f, false);
			ImGui::EndColumns();
			ImGui::EndChildFrame();

			// Update value handle in case type or name has changed
			value = m_Model->GetPropertySet(propertyType).GetValue(newName);
			isArrayOrVector = value.isArray();

			// Switch between Array vs Value

			ImGui::SameLine(0.0f, 0.0f);
			UI::ShiftCursorY(3);

			if (value.isVoid())
				ImGui::BeginDisabled();

			bool isArray = value.isArray();
			if (ImGui::Checkbox("Is Array", &isArray))
			{
				if (isArray) // Became array
				{
					choc::value::Value arrayValue = choc::value::createEmptyArray();
					arrayValue.addArrayElement(value);

					m_Model->ChangePropertyType(propertyType, newName, arrayValue);
					isArrayOrVector = true;
					value = arrayValue;
				}
				else // Became simple
				{
					auto newValue = choc::value::Value(value.getType().getElementType());

					m_Model->ChangePropertyType(propertyType, newName, newValue);
					isArrayOrVector = false;
					value = newValue;
				}
			}
			if (value.isVoid())
				ImGui::EndDisabled();
			else
				UI::DrawItemActivityOutline(2.5f, true);

			ImGui::Unindent();

			ImGui::Dummy(ImVec2(panelWidth, 10.0f));

			// Assign default value to the Value or Array elements

			auto valueEditField = [](const char* label, choc::value::ValueView& valueView, bool* removeButton = nullptr)
			{
				ImGui::Text(label);
				ImGui::NextColumn();
				ImGui::PushItemWidth(-ImGui::GetFrameHeight());

				bool changed = false;
				const std::string id = "##" + std::string(label);

				if (valueView.isFloat())
				{
					float v = valueView.get<float>();
					if (ImGui::DragFloat(id.c_str(), &v, 0.01f, 0.0f, 0.0f, "%.2f"))
					{
						valueView.set(v);
						changed = true;
					}

				}
				else if (valueView.isInt32())
				{
					int32_t v = valueView.get<int32_t>();
					if (ImGui::DragInt(id.c_str(), &v, 0.1f, 0, 0))
					{
						valueView.set(v);
						changed = true;
					}
				}
				else if (valueView.isBool())
				{
					bool v = valueView.get<bool>();
					if (ImGui::Checkbox(id.c_str(), &v))
					{
						valueView.set(v);
						changed = true;
					}
				}
				else if (valueView.isString())
				{
					std::string v = valueView.get<std::string>();

					static char buffer[128]{};
					memset(buffer, 0, sizeof(buffer));
					memcpy(buffer, v.data(), std::min(v.size(), sizeof(buffer)));

					const auto inputTextFlags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;
					ImGui::InputText(id.c_str(), buffer, 127, inputTextFlags);

					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						valueView.set(std::string(buffer));
						changed = true;
					}
				}
				else if (IsAssetHandle(valueView)) // AssetHandle
				{
					ImGui::PushID((id + "AssetReference").c_str());

					AssetHandle selected = 0;
					Ref<AudioFile> asset;

					selected = GetAssetHandleValue(valueView);

					if (AssetManager::IsAssetHandleValid(selected))
					{
						asset = AssetManager::GetAsset<AudioFile>(selected);
					}
					// TODO: get supported asset type from the pin
					//		Initialize choc::Value class object with correct asset type using helper function
					bool assetDropped = false;
					if (UI::AssetReferenceDropTargetButton(id.c_str(), asset, AssetType::Audio, assetDropped))
					{
						ImGui::OpenPopup((id + "GraphInputWaveAssetPopup").c_str());
					}

					if (assetDropped)
					{
						SetAssetHandleValue(valueView, selected);
						changed = true;
					}

					bool clear = false;
					if (UI::Widgets::AssetSearchPopup((id + "GraphInputWaveAssetPopup").c_str(), AssetType::Audio, selected, &clear))
					{
						if (clear)
						{
							selected = 0;
							SetAssetHandleValue(valueView, 0);
						}
						else
						{
							SetAssetHandleValue(valueView, selected);
						}

						changed = true;
					}

					ImGui::PopID();
				}
				else
				{
					ANT_CORE_ERROR("Invalid type of Graph Input!");
				}

				UI::DrawItemActivityOutline(2.5f, true, Colours::Theme::accent);

				ImGui::PopItemWidth();

				if (removeButton != nullptr)
				{
					ImGui::SameLine();

					ImGui::PushID((id + "removeButton").c_str());

					if (ImGui::SmallButton("x"))
						*removeButton = true;

					ImGui::PopID();
				}

				ImGui::NextColumn();

				return changed;
			};


			ImGui::GetWindowDrawList()->AddRectFilled(
				ImGui::GetCursorScreenPos() - ImVec2(0.0f, 2.0f),
				ImGui::GetCursorScreenPos() + ImVec2(panelWidth, ImGui::GetTextLineHeightWithSpacing()),
				ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]), 2.5f);

			ImGui::Spacing(); ImGui::SameLine(); UI::ShiftCursorY(2.0f);
			ImGui::Text("Default Value");

			ImGui::Spacing();
			ImGui::Dummy(ImVec2(panelWidth, 4.0f));

			ImGui::Indent();

			const bool isTriggerType = value.isVoid();

			if (!isTriggerType && !isArrayOrVector)
			{
				ImGui::Columns(2);
				if (valueEditField("Value", value.getViewReference()))
				{
					m_Model->SetPropertyValue(propertyType, newName, value);
				}
				ImGui::EndColumns();
			}
			else if (!isTriggerType) // Array
			{
				ImGui::BeginHorizontal("ArrayElementsInfo", { ImGui::GetContentRegionAvail().x, 0.0f });
				uint32_t size = value.size();
				ImGui::Text((std::to_string(size) + " Array elements").c_str());

				ImGui::Spring();

				//? For now limiting max number of elements in array to fit into
				//? choc's small vector optimization and to prevent large amount
				//? of data being copied to SoundGraph player
				const bool reachedMaxArraySize = value.size() >= 32;
				if (reachedMaxArraySize)
				{
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
				}

				bool changed = false;
				bool invalidatePlayer = false;

				if (ImGui::SmallButton("Add Element"))
				{
					value.addArrayElement(choc::value::Value(value.getType().getElementType()));

					changed = true;
					invalidatePlayer = true;
				}

				if (reachedMaxArraySize)
				{
					ImGui::PopItemFlag(); // ImGuiItemFlags_Disabled
					ImGui::PopStyleColor(); // ImGuiCol_Text
				}

				ImGui::Spring(0.0f, 4.0f);
				ImGui::EndHorizontal();

				ImGui::Spacing();

				ImGui::Columns(2);

				int indexToRemove = -1;
				UI::PushID();
				for (uint32_t i = 0; i < value.size(); ++i)
				{
					choc::value::ValueView vv = value[i];

					bool removeThis = false;
					if (valueEditField(("[ " + std::to_string(i) + " ]").c_str(), vv, &removeThis))
						changed = true;

					if (value.size() > 1 && removeThis) indexToRemove = i;
				}
				UI::PopID();

				if (indexToRemove >= 0)
				{
					bool skipIteration = false;
					value = choc::value::createArray(value.size() - 1, [&value, &skipIteration, indexToRemove](uint32_t i)
						{
							if (!skipIteration)
								skipIteration = i == indexToRemove;
							return skipIteration ? value[i + 1] : value[i];
						});

					changed = true;
					invalidatePlayer = true;
				}

				if (invalidatePlayer)
					m_Model->InvalidatePlayer();

				if (changed)
				{
					m_Model->SetPropertyValue(propertyType, newName, value);
				}

				ImGui::EndColumns();
			}
			// TODO: invalidate connections that were using this input

			ImGui::Unindent();
		}
	}

	void SoundGraphNodeGraphEditor::DrawGraphIO()
	{
		const float panelWidth = ImGui::GetContentRegionAvail().x;

		//=============================================================================
		/// Inputs & Outputs
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		if (ImGui::BeginChild("Inputs Outputs"))
		{

			auto addInputButton = [&]
			{
				ImGui::SameLine(panelWidth - ImGui::GetFrameHeight() - ImGui::CalcTextSize("+ Output").x);
				if (ImGui::SmallButton("+ Input"))
				{
					m_Model->AddPropertyToGraph(ESoundGraphPropertyType::Input, choc::value::createFloat32(0.0f));
				}
			};

			const int leafFlags = ImGuiTreeNodeFlags_Leaf
				| ImGuiTreeNodeFlags_NoTreePushOnOpen
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| ImGuiTreeNodeFlags_FramePadding
				| ImGuiTreeNodeFlags_AllowItemOverlap;

			const int headerFlags = ImGuiTreeNodeFlags_CollapsingHeader
				| ImGuiTreeNodeFlags_AllowItemOverlap
				| ImGuiTreeNodeFlags_DefaultOpen;

			// INPUTS list
			ImGui::Spacing();
			if (UI::PropertyGridHeader("INPUTS"))
			{
				addInputButton();

				ImGui::TreeNodeEx("Input Action", leafFlags);

				std::string inputToRemove;

				for (auto& input : m_Model->GetInputs().GetNames())
				{
					bool selected = m_SelectedItem->IsSelected(ESoundGraphPropertyType::Input, input);
					int flags = selected ? leafFlags | ImGuiTreeNodeFlags_Selected : leafFlags;

					// Colouring text, just because
					ImColor textColour = GetTypeColour(m_Model->GetInputs().GetValue(input));

					ImGui::PushStyleColor(ImGuiCol_Text, textColour.Value);
					ImGui::TreeNodeEx(input.c_str(), flags);
					ImGui::PopStyleColor();

					if (ImGui::IsItemClicked())
						m_SelectedItem->Select(ESoundGraphPropertyType::Input, input);

					// Remove Input buton
					ImGui::SameLine();
					ImGui::SetCursorPosX(panelWidth - ImGui::GetFrameHeight());

					ImGui::PushID((input + "removeInput").c_str());
					if (ImGui::SmallButton("x"))
						inputToRemove = input;
					ImGui::PopID();
				}

				if (!inputToRemove.empty())
				{
					if (inputToRemove == m_SelectedItem->Name)
						m_SelectedItem->Clear();

					m_Model->RemovePropertyFromGraph(ESoundGraphPropertyType::Input, inputToRemove);
				}

				ImGui::TreePop();
			}
			else
			{
				addInputButton();
			}

			auto addOutputButton = [&]
			{
				ImGui::SameLine(panelWidth - ImGui::GetFrameHeight() - ImGui::CalcTextSize("+ Output").x);
				if (ImGui::SmallButton("+ Output"))
				{
					// TODO: add output
					ANT_CORE_WARN("Add Output to Graph not implemented.");
				}
			};

			// OUTPUTS list
			if (UI::PropertyGridHeader("OUTPUTS"))
			{
				addOutputButton();

				ImGui::TreeNodeEx("Output Audio", leafFlags);

				ImGui::TreePop();
			}
			else
			{
				addOutputButton();
			}

			auto addVariableButton = [&]
			{
				ImGui::SameLine(panelWidth - ImGui::GetFrameHeight() - ImGui::CalcTextSize("+ Variable").x);
				if (ImGui::SmallButton("+ Variable"))
				{
					m_Model->AddPropertyToGraph(ESoundGraphPropertyType::LocalVariable, choc::value::createFloat32(0.0f));
				}
			};

			// Variables / Reroutes
			if (UI::PropertyGridHeader("LOCAL VARIABLES"))
			{
				addVariableButton();

				std::string variableToRemove;
				for (auto& input : m_Model->GetLocalVariables().GetNames())
				{
					bool selected = m_SelectedItem->IsSelected(ESoundGraphPropertyType::LocalVariable, input);
					int flags = selected ? leafFlags | ImGuiTreeNodeFlags_Selected : leafFlags;

					// Colouring text, just because
					ImColor textColour = GetTypeColour(m_Model->GetLocalVariables().GetValue(input));

					ImGui::PushStyleColor(ImGuiCol_Text, textColour.Value);
					ImGui::TreeNodeEx(input.c_str(), flags);
					ImGui::PopStyleColor();

					if (ImGui::IsItemClicked())
						m_SelectedItem->Select(ESoundGraphPropertyType::LocalVariable, input);

					// Remove Input buton
					ImGui::SameLine();
					ImGui::SetCursorPosX(panelWidth - ImGui::GetFrameHeight());

					ImGui::PushID((input + "removeVariable").c_str());
					if (ImGui::SmallButton("x"))
						variableToRemove = input;
					ImGui::PopID();
				}

				if (!variableToRemove.empty())
				{
					if (variableToRemove == m_SelectedItem->Name)
						m_SelectedItem->Clear();

					m_Model->RemovePropertyFromGraph(ESoundGraphPropertyType::LocalVariable, variableToRemove);
				}
				ImGui::TreePop();
			}
			else
			{
				addVariableButton();
			}
		}
		ImGui::EndChild();

		ImGui::PopStyleVar();

		//=============================================================================
		/// Details

		ImGui::SetNextWindowClass(GetWindowClass());
		if (ImGui::Begin("Details##SoundGraphNodeGraph"))
		{
			if (ed::HasSelectionChanged())
			{
				std::vector<UUID> selectedNodes = GetSelectedNodes();
				if (selectedNodes.size() == 1)
				{
					m_SelectedItem->Type = SelectedItem::EType::Node;
					m_SelectedItem->NodeID = selectedNodes.back();
				}
				else if (m_SelectedItem->Type == SelectedItem::EType::Node)
				{
					m_SelectedItem->Clear();
				}
			}

			if (*m_SelectedItem)
			{
				if (m_SelectedItem->Type == SelectedItem::EType::Node)
					DrawNodeDetails(m_SelectedItem->NodeID);
				else
					DrawPropertyDetails(m_SelectedItem->GetPropertyType(), m_SelectedItem->Name);
			}

			//? DBG Info
			ImGui::Dummy(ImVec2(panelWidth, 40.0f));
			ImGui::BeginHorizontal("##debug");
			ImGui::Checkbox("Show IDs", &m_ShowNodeIDs);
			ImGui::Checkbox("Show Sort Indices", &m_ShowSortIndices);
			ImGui::EndHorizontal();
		}
		ImGui::End();
	}

	ImColor SoundGraphNodeGraphEditor::GetTypeColour(const choc::value::Value& v)
	{
		const Nodes::SGTypes::ESGPinType pinType = Nodes::SGTypes::GetPinTypeForValue(v);
		return  Nodes::SGTypes::GetPinColour(pinType);
	}

	std::pair<Nodes::SGTypes::ESGPinType, StorageKind> SoundGraphNodeGraphEditor::GetPinTypeAndStorageKindForValue(const choc::value::Value& v)
	{
		const Nodes::SGTypes::ESGPinType pinType = Nodes::SGTypes::GetPinTypeForValue(v);
		const bool isArray = v.isArray();
		return { pinType, isArray ? StorageKind::Array : StorageKind::Value };
	}

} // namespace Ant