#include "antpch.h"
#include "NodeEditorModel.h"

#include "Ant/Asset/AssetManager.h"

#include "choc/text/choc_StringUtilities.h"

#include <sstream>

namespace Ant
{
	//=============================================================================
	/// IDs

#pragma region IDs

	uintptr_t NodeEditorModel::GetIDFromString(const std::string& idString) const
	{
		std::stringstream stream(idString);
		uintptr_t ret;
		stream >> ret;
		return ret;
	}

	void NodeEditorModel::AssignIds(Node* node)
	{
		node->ID = UUID();

		for (auto& input : node->Inputs)
			input->ID = UUID();

		for (auto& output : node->Outputs)
			output->ID = UUID();

	}

	void NodeEditorModel::OnNodeSpawned(Node* node)
	{
		if (node)
		{
			AssignIds(node);
			GetNodes().push_back(node);
		}
	}

#pragma endregion IDs


	//=============================================================================
	/// Find items

#pragma region Find objects

	Node* NodeEditorModel::FindNode(UUID id)
	{
		for (auto& node : GetNodes())
			if (node->ID == id)
				return node;

		return nullptr;
	}

	const Node* NodeEditorModel::FindNode(UUID id) const
	{
		for (auto& node : GetNodes())
			if (node->ID == id)
				return node;

		return nullptr;
	}

	Link* NodeEditorModel::FindLink(UUID id)
	{
		for (auto& link : GetLinks())
			if (link.ID == id)
				return &link;

		return nullptr;
	}

	const Link* NodeEditorModel::FindLink(UUID id) const
	{
		for (auto& link : GetLinks())
			if (link.ID == id)
				return &link;

		return nullptr;
	}

	Pin* NodeEditorModel::FindPin(UUID id)
	{
		if (!id)
			return nullptr;

		for (auto& node : GetNodes())
		{
			for (auto& pin : node->Inputs)
				if (pin->ID == id)
					return pin;

			for (auto& pin : node->Outputs)
				if (pin->ID == id)
					return pin;
		}

		return nullptr;
	}

	const Pin* NodeEditorModel::FindPin(UUID id) const
	{
		if (!id)
			return nullptr;

		for (auto& node : GetNodes())
		{
			for (auto& pin : node->Inputs)
				if (pin->ID == id)
					return pin;

			for (auto& pin : node->Outputs)
				if (pin->ID == id)
					return pin;
		}

		return nullptr;
	}

#pragma endregion Find items


	//=============================================================================
	/// Links

#pragma region Links

	Link* NodeEditorModel::GetLinkConnectedToPin(UUID id)
	{
		if (!id)
			return nullptr;

		for (auto& link : GetLinks())
			if (link.StartPinID == id || link.EndPinID == id)
				return &link;

		return nullptr;
	}

	const Link* NodeEditorModel::GetLinkConnectedToPin(UUID id) const
	{
		if (!id)
			return nullptr;

		for (auto& link : GetLinks())
			if (link.StartPinID == id || link.EndPinID == id)
				return &link;

		return nullptr;
	}

	const Node* NodeEditorModel::GetNodeForPin(UUID pinID) const
	{
		if (!pinID)
			return nullptr;

		for (const auto& node : GetNodes())
		{
			for (auto& pin : node->Inputs)
				if (pin->ID == pinID)
					return node;

			for (auto& pin : node->Outputs)
				if (pin->ID == pinID)
					return node;
		}

		return nullptr;
	}

	const Node* NodeEditorModel::GetNodeConnectedToPin(UUID pinID) const
	{
		const Pin* pin = FindPin(pinID);
		if (!pin) return nullptr;
		const Link* link = GetLinkConnectedToPin(pin->ID);
		if (!link) return nullptr;
		const Pin* otherPin = FindPin(pin->Kind == PinKind::Input ? link->StartPinID : link->EndPinID);
		if (!otherPin) return nullptr;

		return FindNode(otherPin->NodeID);
	}

	Node* NodeEditorModel::GetNodeConnectedToPin(UUID pinID)
	{
		Pin* pin = FindPin(pinID);
		if (!pin) return nullptr;
		Link* link = GetLinkConnectedToPin(pin->ID);
		if (!link) return nullptr;
		Pin* otherPin = FindPin(pin->Kind == PinKind::Input ? link->StartPinID : link->EndPinID);
		if (!otherPin) return nullptr;

		return FindNode(otherPin->NodeID);
	}

	bool NodeEditorModel::IsPinLinked(UUID pinID)
	{
		return GetLinkConnectedToPin(pinID) != nullptr;
	}

	bool NodeEditorModel::IsPinLinked(UUID pinID) const
	{
		return GetLinkConnectedToPin(pinID) != nullptr;
	}

	std::vector<Link*> NodeEditorModel::GetAllLinksConnectedToPin(UUID pinID)
	{
		if (!pinID)
			return {};

		std::vector<Link*> links;

		for (auto& link : GetLinks())
			if (link.StartPinID == pinID || link.EndPinID == pinID)
				links.push_back(&link);

		return links;
	}

	std::vector<UUID> NodeEditorModel::GetAllLinkIDsConnectedToPin(UUID pinID)
	{
		if (!pinID)
			return {};

		std::vector<UUID> links;

		for (auto& link : GetLinks())
			if (link.StartPinID == pinID || link.EndPinID == pinID)
				links.push_back(link.ID);

		return links;
	}

	void NodeEditorModel::DeleteDeadLinks(UUID nodeId)
	{
		auto wasConnectedToTheNode = [&](const Link& link)
		{
			return (!FindPin(link.StartPinID)) || (!FindPin(link.EndPinID))
				|| FindPin(link.StartPinID)->NodeID == nodeId
				|| FindPin(link.EndPinID)->NodeID == nodeId;
		};

		auto& links = GetLinks();

		// Clear pin value
		std::for_each(links.begin(), links.end(), [&](Link& link)
			{
				if (FindPin(link.StartPinID)->NodeID == nodeId)
				{
					if (auto* pin = FindPin(link.EndPinID))
						pin->Value = choc::value::Value();
				}

			});

		auto removeIt = std::remove_if(links.begin(), links.end(), wasConnectedToTheNode);
		const bool linkRemoved = removeIt != links.end();

		links.erase(removeIt, links.end());

		if (linkRemoved)
			OnLinkDeleted();
	}

	void NodeEditorModel::AssignSomeDefaultValue(Pin* pin)
	{
		if (pin->Kind == PinKind::Output) // implementation specific
			return;

		// Reset to some sort of valid default value
		switch (pin->GetType())
		{
			case EPinType::Bool:		pin->Value = choc::value::Value(true); break;
			case EPinType::Float:	pin->Value = choc::value::Value(1.0f); break;
			case EPinType::Int:		pin->Value = choc::value::Value(int(0)); break;
			case EPinType::Enum:		pin->Value = choc::value::Value(int(0)); break;
			case EPinType::Object:	pin->Value = choc::value::Value(int64_t(0)); break;
			case EPinType::String:	pin->Value = choc::value::Value(""); break;
			default: break;
		}
	}

	NodeEditorModel::LinkQueryResult NodeEditorModel::CanCreateLink(Pin* startPin, Pin* endPin)
	{
		if (!startPin)								return LinkQueryResult::InvalidStartPin;
		else if (!endPin)                               return LinkQueryResult::InvalidEndPin;
		else if (endPin == startPin)                    return LinkQueryResult::SamePin;
		else if (endPin->NodeID == startPin->NodeID)    return LinkQueryResult::SameNode;
		else if (endPin->Kind == startPin->Kind)        return LinkQueryResult::IncompatiblePinKind;
		else if (endPin->Storage != startPin->Storage)  return LinkQueryResult::IncompatibleStorageKind;
		else if (!endPin->IsSameType(startPin))			return LinkQueryResult::IncompatibleType;

		return LinkQueryResult::CanConnect;
	}

	void NodeEditorModel::CreateLink(Pin* startPin, Pin* endPin)
	{
		ANT_CORE_ASSERT(startPin && endPin);

		auto& links = GetLinks();

		links.emplace_back(Link(UUID(), startPin->ID, endPin->ID));

		// TODO: get static Types factory from CRTP type and get icon colour from it?
		links.back().Color = GetIconColor(startPin->GetType());

		// Clear pin value
		endPin->Value = choc::value::Value(); //? this is not ideal, as it clears the type as well

		OnLinkCreated(links.back().ID);
	}

	void NodeEditorModel::RemoveLink(UUID linkId)
	{
		auto& links = GetLinks();

		auto id = std::find_if(links.begin(), links.end(), [linkId](auto& link) { return link.ID == linkId; });
		if (id != links.end())
		{
			// Clear pin value
			if (auto* endPin = FindPin(id->EndPinID))
				AssignSomeDefaultValue(endPin);

			links.erase(id);

			OnLinkDeleted();
		}
	}

	void NodeEditorModel::RemoveLinks(const std::vector<UUID>& linkIds)
	{
		for (const auto& linkId : linkIds)
			RemoveLink(linkId);
	}

#pragma endregion Links


	//=============================================================================
	/// Nodes

#pragma region Nodes

	void NodeEditorModel::RemoveNode(UUID nodeId)
	{
		auto& nodes = GetNodes();

		auto id = std::find_if(nodes.begin(), nodes.end(), [nodeId](auto& node) { return node->ID == nodeId; });

		if (id != nodes.end())
		{
			DeleteDeadLinks(nodeId);
			nodes.erase(id);

			// When a node is deleted not from the end of the vector,
			// nodes after it are moved back, which changes their address.
			// In this case we need to rebuild nodes, i.e. assign new
			// pointers to owner nodes to each pin.
			BuildNodes();

			OnNodeDeleted();
		}
	}

	void NodeEditorModel::RemoveNodes(const std::vector<UUID>& nodeIds)
	{
		auto& nodes = GetNodes();

		for (const auto& id : nodeIds)
			DeleteDeadLinks(id);

		auto it = std::remove_if(nodes.begin(), nodes.end(), [&nodeIds](auto& node)
			{
				for (const auto& id : nodeIds)
				{
					if (node->ID == id)
						return true;
				}
				return false;
			});

		if (it != nodes.end())
		{
			nodes.erase(it, nodes.end());

			// When a node is deleted not from the end of the vector,
			// nodes after it are moved back, which changes their address.
			// In this case we need to rebuild nodes, i.e. assign new
			// pointers to owner nodes to each pin.
			BuildNodes();

			OnNodeDeleted();
		}
	}

	void NodeEditorModel::BuildNode(Node* node)
	{
		for (auto& input : node->Inputs)
		{
			input->NodeID = node->ID;
			input->Kind = PinKind::Input;

			// clean up after deleted links
			if (input->Value.isVoid() && !IsPinLinked(input->ID))
				AssignSomeDefaultValue(input);
		}

		for (auto& output : node->Outputs)
		{
			output->NodeID = node->ID;
			output->Kind = PinKind::Output;
		}
	}

	void NodeEditorModel::BuildNodes()
	{
		for (auto* node : GetNodes())
			BuildNode(node);
	}

	Node* NodeEditorModel::CreateNode(const std::string& category, const std::string& typeID)
	{
		//std::string safeTypeName = choc::text::replace(typeID, " ", "_");
		//std::string safeCategory = choc::text::replace(category, " ", "_");

		if (auto* newNode = GetNodeFactory()->SpawnNode(category, typeID))
		{
			OnNodeSpawned(newNode);
			//BuildNode(&GetNodes().back());
			// std::vector of nodes migh have reallocated when resized to add new node,
			// so pointers might have been invalidated
			BuildNodes();

			OnNodeCreated();

			return GetNodes().back();
		}
		else
		{
			return nullptr;
		}
	}

#pragma endregion Nodes


	//=============================================================================
	/// Serialization

#pragma region Serialization

	void NodeEditorModel::SaveAll()
	{
		Serialize();
	}

	void NodeEditorModel::LoadAll()
	{
		Deserialize();
		BuildNodes();
	}

#pragma endregion Serialization


	//=============================================================================
	/// Model Interface

	void NodeEditorModel::CompileGraph()
	{
		OnCompileGraph();
	}

} // namespace Ant