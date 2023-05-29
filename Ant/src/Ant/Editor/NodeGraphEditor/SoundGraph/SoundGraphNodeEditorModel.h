#pragma once

#include "SoundGraphNodes.h"

#include "Ant/Editor/NodeGraphEditor/NodeEditorModel.h"
#include "Ant/Editor/NodeGraphEditor/PropertySet.h"

namespace Ant {
	class SoundGraphSource;
	class SoundGraphCache;

	//==================================================================================
	/// SoundGraph Graph Editor Model
	class SoundGraphNodeEditorModel : public NodeEditorModel
	{
	public:
		explicit SoundGraphNodeEditorModel(Ref<SoundGraphAsset> graphAsset);
		~SoundGraphNodeEditorModel();

		std::vector<Node*>& GetNodes() override { return m_GraphAsset->Nodes; }
		std::vector<Link>& GetLinks() override { return m_GraphAsset->Links; }
		const std::vector<Node*>& GetNodes() const override { return m_GraphAsset->Nodes; }
		const std::vector<Link>& GetLinks() const override { return m_GraphAsset->Links; }

		Utils::PropertySet& GetInputs() { return m_GraphAsset->GraphInputs; }
		Utils::PropertySet& GetOutputs() { return m_GraphAsset->GraphOutputs; }
		Utils::PropertySet& GetLocalVariables() { return m_GraphAsset->LocalVariables; }

		const Nodes::Registry& GetNodeTypes() const override { return m_NodeFactory.Registry; }

		static std::string GetPropertyToken(ESoundGraphPropertyType type);
		static ESoundGraphPropertyType GetPropertyType(std::string_view propertyToken);
		static bool IsPropertyNode(ESoundGraphPropertyType type, const Node* node, std::string_view propertyName);
		static bool IsLocalVariableNode(const Node* node);
		static bool IsSameLocalVariableNodes(const Node* nodeA, const Node* nodeB);
		Pin* FindLocalVariableSource(std::string_view localVariableName);

		ESoundGraphPropertyType GetPropertyTypeOfNode(const Node* node);
		/** @returns true if any of the property sets contain property with the name of the node or its out/in pin. */
		bool IsPropertyNode(const Node* node) const;

		Utils::PropertySet& GetPropertySet(ESoundGraphPropertyType type);
		const Utils::PropertySet& GetPropertySet(ESoundGraphPropertyType type) const;

		void AddPropertyToGraph(ESoundGraphPropertyType type, const choc::value::ValueView& value);
		void RemovePropertyFromGraph(ESoundGraphPropertyType type, const std::string& name);
		void SetPropertyValue(ESoundGraphPropertyType type, const std::string& propertyName, const choc::value::ValueView& value);
		void RenameProperty(ESoundGraphPropertyType type, const std::string& oldName, const std::string& newName);
		void ChangePropertyType(ESoundGraphPropertyType type, const std::string& propertyName, const choc::value::ValueView& valueOfNewType);

		void OnGraphPropertyNameChanged(ESoundGraphPropertyType type, const std::string& oldName, const std::string& newName);
		void OnGraphPropertyTypeChanged(ESoundGraphPropertyType type, const std::string& inputName);
		void OnGraphPropertyValueChanged(ESoundGraphPropertyType type, const std::string& inputName);

		// TODO: SpawnGraphOutputNode
		Node* SpawnGraphInputNode(const std::string& inputName);
		Node* SpawnLocalVariableNode(const std::string& inputName, bool getter);

		uint64_t GetCurrentPlaybackFrame() const;
		bool IsPlayerDirty() const noexcept { return m_PlayerDirty; }
		// Should be called to update sound source
		void OnUpdate(Timestep ts);

		/** Flag player dirty. Dirty player needs to be recompiled
			and parameters changes can't be applied live.
		*/
		void InvalidatePlayer();

		bool IsGraphPropertyNameValid(ESoundGraphPropertyType type, std::string_view newName) const noexcept;


	private:
		// Sort nodes in order that makes sense
		bool Sort() override;
		void Serialize() override { Sort(); AssetImporter::Serialize(m_GraphAsset); };
		void Deserialize() override;

		bool SaveGraphState(const char* data, size_t size) override;
		const std::string& LoadGraphState() override;

		const Nodes::AbstractFactory* GetNodeFactory() const override { return &m_NodeFactory; }

		LinkQueryResult CanCreateLink(Pin* startPin, Pin* endPin) override;

		void OnCompileGraph() override;

		void OnNodeCreated() final;
		void OnNodeDeleted() final;
		void OnLinkCreated(UUID linkID) final;
		void OnLinkDeleted() final;

		void AssignSomeDefaultValue(Pin* pin) final;

	public:
		std::function<void()> onCompiledSuccessfully = nullptr;

	private:
		Nodes::SoundGraphNodeFactory m_NodeFactory;
		Ref<SoundGraphAsset> m_GraphAsset = nullptr;
		Scope<SoundGraphSource> m_SoundGraphSource = nullptr;
		bool m_PlayerDirty = true;

		Scope<SoundGraphCache> m_Cache = nullptr;
	};

} // namespace Ant
