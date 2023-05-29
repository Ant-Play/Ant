#pragma once

#include "Ant/Editor/NodeGraphEditor/NodeGraphEditor.h"
#include "Ant/Editor/NodeGraphEditor/SoundGraph/SoundGraphEditorTypes.h"

namespace Ant
{
	//==================================================================================
	/// SoundGraph Node Editor
	class SoundGraphNodeEditorModel;

	class SoundGraphNodeGraphEditor final : public NodeGraphEditorBase
	{
	public:
		SoundGraphNodeGraphEditor();
		~SoundGraphNodeGraphEditor();

		void SetAsset(const Ref<Asset>& asset) override;

		static ImColor GetTypeColour(const choc::value::Value& v);
		static std::pair<Nodes::SGTypes::ESGPinType, StorageKind> GetPinTypeAndStorageKindForValue(const choc::value::Value& v);

	private:
		ImGuiWindowFlags GetWindowFlags() override;
		void OnWindowStylePush() override;
		void OnWindowStylePop() override;

		void OnUpdate(Timestep ts) override;
		void OnRender() override;
		void OnRenderOnCanvas(ImVec2 min, ImVec2 max) override;

		void OnClose() override;

		const char* GetIconForSimpleNode(const std::string& simpleNodeIdentifier) const override;
		void DrawPinIcon(const Pin* pin, bool connected, int alpha) override;

		bool DrawPinPropertyEdit(PinPropertyContext& context) override;


		void DrawToolbar();
		void DrawDetailsPanel();

		void DrawNodeDetails(UUID nodeID);
		void DrawPropertyDetails(uint8_t propertyType, std::string_view propertyName);
		void DrawGraphIO();

		NodeEditorModel* GetModel() override;
		const NodeEditorModel* GetModel() const override;

	private:
		std::unique_ptr<SoundGraphNodeEditorModel> m_Model = nullptr;

		struct SelectedItem;
		Scope<SelectedItem> m_SelectedItem = nullptr;
	};

} // namespace Ant
