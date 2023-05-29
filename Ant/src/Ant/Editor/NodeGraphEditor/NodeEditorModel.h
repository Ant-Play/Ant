#pragma once

#include "Nodes.h"
#include "NodeGraphAsset.h"

#include "choc/containers/choc_Value.h"

#include "Ant/Asset/AssetImporter.h"

#include <functional>

namespace Ant {

	class NodeEditorModel
	{
	public:
		NodeEditorModel(const NodeEditorModel&) = delete;
		NodeEditorModel operator=(const NodeEditorModel&) = delete;

		NodeEditorModel() = default;
		virtual ~NodeEditorModel() = default;

		//==================================================================================
		/// Graph Model Interface
		virtual std::vector<Node*>& GetNodes() = 0;
		virtual std::vector<Link>& GetLinks() = 0;
		virtual const std::vector<Node*>& GetNodes() const = 0;
		virtual const std::vector<Link>& GetLinks() const = 0;

		virtual const Nodes::Registry& GetNodeTypes() const = 0;

		//==================================================================================
		/// Graph Model
		uintptr_t GetIDFromString(const std::string& idString) const;

		ImColor GetIconColor(int pinTypeID) const { return GetNodeFactory()->GetIconColor(pinTypeID); }

		Node* FindNode(UUID id);
		Link* FindLink(UUID id);
		Pin* FindPin(UUID id);
		const Node* FindNode(UUID id) const;
		const Link* FindLink(UUID id) const;
		const Pin* FindPin(UUID id)   const;

		Link* GetLinkConnectedToPin(UUID pinID);
		const Link* GetLinkConnectedToPin(UUID pinID) const;
		const Node* GetNodeForPin(UUID pinID) const;
		const Node* GetNodeConnectedToPin(UUID pinID) const;	// Get node on the other side of the link
		Node* GetNodeConnectedToPin(UUID pinID);				// Get node on the other side of the link
		bool IsPinLinked(UUID pinID);
		bool IsPinLinked(UUID pinID) const;

		std::vector<Link*> GetAllLinksConnectedToPin(UUID pinID);
		std::vector<UUID> GetAllLinkIDsConnectedToPin(UUID pinID);

		struct LinkQueryResult
		{
			enum
			{
				CanConnect,
				InvalidStartPin,
				InvalidEndPin,
				IncompatiblePinKind,
				IncompatibleStorageKind,
				IncompatibleType,
				SamePin,
				SameNode,
				CausesLoop
			} Reason;

			LinkQueryResult(decltype(Reason) value) : Reason(value) {}

			explicit operator bool()
			{
				return Reason == LinkQueryResult::CanConnect;
			}
		};
		virtual LinkQueryResult CanCreateLink(Pin* startPin, Pin* endPin);
		void CreateLink(Pin* startPin, Pin* endPin);
		void RemoveLink(UUID linkId);
		void RemoveLinks(const std::vector<UUID>& linkIds);

		Node* CreateNode(const std::string& category, const std::string& typeID);
		void  RemoveNode(UUID nodeId);
		void  RemoveNodes(const std::vector<UUID>& nodeIds);

		void SaveAll();
		void LoadAll();
		virtual bool SaveGraphState(const char* data, size_t size) = 0;
		virtual const std::string& LoadGraphState() = 0;

		void CompileGraph();

	private:
		//==================================================================================
		/// Internal
		void AssignIds(Node* node);
		void DeleteDeadLinks(UUID nodeId);

		// Used when deleting links to reassign some sort of valid default value
		virtual void AssignSomeDefaultValue(Pin* pin);

	protected:
		// Adds node to the list of nodes
		void OnNodeSpawned(Node* node);
		// Assigns relationship between pins and nodes
		void BuildNode(Node* node);
		void BuildNodes();

		// Sort nodes in order that makes sense
		virtual bool Sort() { return false; }

		//==================================================================================
		/// Graph Model Interface
	public:
		using NodeID = UUID;
		using PinID = UUID;
		using LinkID = UUID;

		std::function<void()> onNodeCreated = nullptr, onNodeDeleted = nullptr;
		std::function<void(UUID linkID)> onLinkCreated = nullptr;
		std::function<void()> onLinkDeleted = nullptr;

		// This is called from the node graph editor
		std::function<void(NodeID, PinID)> onPinValueChanged = nullptr;

		std::function<bool(Ref<Asset>& graphAsset)> onCompile = nullptr;
		std::function<void()> onPlay = nullptr;
		std::function<void(bool onClose)> onStop = nullptr;

	protected:
		virtual void OnNodeCreated() { if (onNodeCreated) onNodeCreated(); }
		virtual void OnNodeDeleted() { if (onNodeDeleted) onNodeDeleted(); }
		virtual void OnLinkCreated(UUID linkID) { if (onLinkCreated) onLinkCreated(linkID); }
		virtual void OnLinkDeleted() { if (onLinkDeleted) onLinkDeleted(); }

		virtual void OnCompileGraph() {};

	private:
		virtual void Serialize() = 0;
		virtual void Deserialize() = 0;

		virtual const Nodes::AbstractFactory* GetNodeFactory() const = 0;

	};

	inline bool operator==(NodeEditorModel::LinkQueryResult a, NodeEditorModel::LinkQueryResult b)
	{
		return a.Reason == b.Reason;
	}

	inline bool operator!=(NodeEditorModel::LinkQueryResult a, NodeEditorModel::LinkQueryResult b)
	{
		return !(a == b);
	}

} // namespace Ant
