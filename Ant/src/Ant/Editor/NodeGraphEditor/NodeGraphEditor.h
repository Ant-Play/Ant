#pragma once

#include "Nodes.h"
#include "Ant/Editor/AssetEditorPanel.h"

namespace ax::NodeEditor {

	struct Style;
	struct EditorContext;

	namespace Utilities {

		struct BlueprintNodeBuilder; // TODO: rename it and rewrite it to use Ant::Texture2D
	}
}

namespace Ant {

	class NodeEditorModel;

	class NodeEditorDraw
	{
	public:
		static bool PropertyBool(choc::value::Value& value);
		static bool PropertyFloat(choc::value::Value& value);
		static bool PropertyInt(choc::value::Value& value);
		static bool PropertyString(choc::value::Value& value);
		static bool PropertyObject(choc::value::Value& value, Pin* inputPin, bool& openAssetPopup, NodeEditorModel* model);
		static bool PropertyEnum(int enumValue, Pin* pin, bool& openEnumPopup,
			std::function<choc::value::Value(int)> constructValue = [](int selected) { return choc::value::createInt32(selected); });
	};

	class NodeGraphEditorBase : public AssetEditor
	{
	public:
		NodeGraphEditorBase(const char* id);
		virtual ~NodeGraphEditorBase() = default;

		// If you override this, make sure to call base class method from subclass.
		virtual void SetAsset(const Ref<Asset>& asset);
		Ref<Asset> GetAsset() { return m_GraphAsset; }

		virtual void OnUpdate(Timestep ts) override {}
		virtual void OnEvent(Event& e) override {}

		std::vector<UUID> GetSelectedNodes() const;

		//==================================================================================
		/// AssetHandle choc::Value utilities
		static bool IsAssetHandle(const choc::value::ValueView& v) { return v.isInt64() || v.isObjectWithClassName(type::type_name<UUID>()); }
		static bool IsAssetHandle(const choc::value::Type& t) { return t.isInt64() || t.isObjectWithClassName(type::type_name<UUID>()); }
		static uint64_t GetAssetHandleValue(const choc::value::ValueView& v);
		static void SetAssetHandleValue(choc::value::Value& v, uint64_t value);
		static void SetAssetHandleValue(choc::value::ValueView& v, uint64_t value);

	private:
		bool InitializeEditor();

		void Render() override final;

		// Called after drawing main Canvas
		virtual void OnRender() {};
		// Called before ending "Canvas" window
		virtual void OnRenderOnCanvas(ImVec2 min, ImVec2 max) {};

		virtual void DrawPinIcon(const Pin* pin, bool connected, int alpha);
		virtual void DrawNodes();

		virtual void DrawNodeContextMenu(Node* node);
		virtual void DrawPinContextMenu(Pin* pin);
		virtual void DrawLinkContextMenu(Link* link);

		virtual NodeEditorModel* GetModel() = 0;
		virtual const NodeEditorModel* GetModel() const = 0;
		virtual const char* GetIconForSimpleNode(const std::string& simpleNodeIdentifier) const { return nullptr; }

	protected:
		void OnOpen() override;
		void OnClose() override; // If you override this, make sure to call base class method from subclass.

		virtual void InitializeEditorStyle(ax::NodeEditor::Style& style);
		virtual int GetPinIconSize() { return 24; }

		struct PinPropertyContext
		{
			Pin* pin = nullptr;
			NodeEditorModel* model = nullptr;
			bool OpenAssetPopup = false;
			bool OpenEnumPopup = false;
		};
		virtual bool DrawPinPropertyEdit(PinPropertyContext& context);

		bool IsPinAcceptingLink(const Pin* pin) { return m_AcceptingLinkPins.first == pin || m_AcceptingLinkPins.second == pin; }
		ImColor GetIconColor(int pinTypeID) const;

		ImGuiWindowClass* GetWindowClass() { return &m_WindowClass; }
		void EnsureWindowIsDocked(ImGuiWindow* childWindow);

	protected:
		ax::NodeEditor::EditorContext* m_Editor = nullptr;
		bool m_Initialized = false;

		Ref<Asset> m_GraphAsset = nullptr;

		// If subclass graph editor has any custom nodes,
		// the popup UI items for them can be added on this callback
		std::function<Node* (bool searching, std::string_view searchedString)> onNodeListPopup = nullptr;

		// A pair of nodes that are accepting a link connection in this frame
		std::pair<Pin*, Pin*> m_AcceptingLinkPins{ nullptr, nullptr };

		// Render local states
		struct ContextState;
		std::unique_ptr<ContextState> m_State = nullptr;

		// For debugging
		bool m_ShowNodeIDs = false;
		bool m_ShowSortIndices = false;

	private:
		ImGuiWindowClass m_WindowClass;
		ImGuiID m_MainDockID;
		std::unordered_map<ImGuiID, ImGuiID> m_DockIDs;

		std::string m_GraphStatePath;
	};

} // namespace Ant