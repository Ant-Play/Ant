#include "antpch.h"
#include "GraphSerializer.h"
#include "Ant/Editor/NodeGraphEditor/Nodes.h"
#include "Ant/Editor/NodeGraphEditor/NodeGraphAsset.h"
#include "Ant/Editor/NodeGraphEditor/SoundGraph/SoundGraphEditorTypes.h"
#include "AssetManager.h"

#include "Ant/Utilities/YAMLSerializationHelpers.h"

//? not ideal, but need access to the EnumRegistry, alternatively could add
//? virtual GetEnumTokens callback to GraphSerializer class
#include "Ant/Editor/NodeGraphEditor/SoundGraph/SoundGraphNodes.h"

#include "Ant/Utilities/SerializationMacros.h"

#include "choc/text/choc_JSON.h"
#include "magic_enum.hpp"

#include "Ant/Audio/SoundGraph/Utils/SoundGraphCache.h"

static constexpr auto MAX_NUM_CACHED_GRAPHS = 256; // TODO: get this value from somewhere reasonable

namespace Ant{

	std::string PinTypeToString(EPinType pinType)
	{
		return std::string(magic_enum::enum_name<EPinType>(pinType));
	}

	EPinType PinTypeFromString(const std::string_view& pinTypeStr)
	{
		if (auto optValue = magic_enum::enum_cast<EPinType>(pinTypeStr))
			return *optValue;

		ANT_CORE_ASSERT(false, "Unknown Pin Type");
		return EPinType::Flow;
	}

	std::string StorageKindToString(StorageKind storageKind)
	{
		return std::string(magic_enum::enum_name<StorageKind>(storageKind));
	}

	StorageKind StorageKindFromString(const std::string_view& storageKindStr)
	{
		if (auto optValue = magic_enum::enum_cast<StorageKind>(storageKindStr))
			return *optValue;

		ANT_CORE_ASSERT(false, "Unknown Storage Kind");
		return StorageKind::Value;
	}

	std::string NodeTypeToString(NodeType nodeType)
	{
		return std::string(magic_enum::enum_name<NodeType>(nodeType));
	}

	NodeType NodeTypeFromString(const std::string_view& nodeTypeStr)
	{
		if (auto optValue = magic_enum::enum_cast<NodeType>(nodeTypeStr))
			return *optValue;

		ANT_CORE_ASSERT(false, "Unknown Node Type");
		return NodeType::Simple;
	}

	//! Old Node Graph serialization, kept as an example
#if 0
	void DefaultGraphSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<SoundGraphAsset> graph = asset.As<SoundGraphAsset>();

		// Out
		std::ofstream fout(Project::GetEditorAssetManager()->GetFileSystemPath(metadata));
		ANT_CORE_VERIFY(fout.good());
		std::string yamlString = SerializeToYAML(graph);
		fout << yamlString;
	}

	bool DefaultGraphSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(Project::GetEditorAssetManager()->GetFileSystemPath(metadata));
		if (!stream.is_open())
			return false;

		std::stringstream strStream;
		strStream << stream.rdbuf();

		Ref<SoundGraphAsset> soundGraphAsset;
		if (!DeserializeFromYAML(strStream.str(), soundGraphAsset))
			return false;

		asset = soundGraphAsset;
		asset->Handle = metadata.Handle;
		return true;
	}

	bool DefaultGraphSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		Ref<SoundGraphAsset> soundGraph = AssetManager::GetAsset<SoundGraphAsset>(handle);

		std::string yamlString = SerializeToYAML(soundGraph);
		outInfo.Offset = stream.GetStreamPosition();
		stream.WriteString(yamlString);
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		return true;
	}

	Ref<Asset> DefaultGraphSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		ANT_CORE_VERIFY(false, "DefaultGraphSerialized should not be used.");

		stream.SetStreamPosition(assetInfo.PackedOffset);
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<SoundGraphAsset> soundGraph;
		bool result = DeserializeFromYAML(yamlString, soundGraph);
		if (!result)
			return nullptr;

		return soundGraph;
	}
#endif

	void DefaultGraphSerializer::SerializeNodes(YAML::Emitter& out, const std::vector<Node*>& nodes)
	{
		out << YAML::Key << "Nodes" << YAML::Value;
		out << YAML::BeginSeq;
		for (const auto& node : nodes)
		{
			out << YAML::BeginMap; // node

			const ImVec4& nodeCol = node->Color.Value;
			const ImVec2& nodeSize = node->Size;
			const glm::vec4 nodeColOut(nodeCol.x, nodeCol.y, nodeCol.z, nodeCol.w);
			const glm::vec2 nodeSizeOut(nodeSize.x, nodeSize.y);

			ANT_SERIALIZE_PROPERTY(ID, node->ID, out);
			ANT_SERIALIZE_PROPERTY(Category, node->Category, out);
			ANT_SERIALIZE_PROPERTY(Name, node->Name, out);
			ANT_SERIALIZE_PROPERTY(Colour, nodeColOut, out);
			ANT_SERIALIZE_PROPERTY(Type, NodeTypeToString(node->Type), out);
			ANT_SERIALIZE_PROPERTY(Size, nodeSizeOut, out);
			ANT_SERIALIZE_PROPERTY(Location, node->State, out);

			out << YAML::Key << "Inputs" << YAML::BeginSeq;
			for (auto& in : node->Inputs)
			{
				out << YAML::BeginMap; // in

				ANT_SERIALIZE_PROPERTY(ID, in->ID, out);
				ANT_SERIALIZE_PROPERTY(Name, in->Name, out);
				ANT_SERIALIZE_PROPERTY(Type, std::string(in->GetTypeString()), out);
				ANT_SERIALIZE_PROPERTY(Storage, StorageKindToString(in->Storage), out);
				ANT_SERIALIZE_PROPERTY(Value, in->Value, out);

				out << YAML::EndMap; // in
			}
			out << YAML::EndSeq; // Inputs

			out << YAML::Key << "Outputs" << YAML::BeginSeq;
			for (auto& outp : node->Outputs)
			{
				out << YAML::BeginMap; // outp
				ANT_SERIALIZE_PROPERTY(ID, outp->ID, out);
				ANT_SERIALIZE_PROPERTY(Name, outp->Name, out);

				ANT_SERIALIZE_PROPERTY(Type, std::string(outp->GetTypeString()), out);
				ANT_SERIALIZE_PROPERTY(Storage, StorageKindToString(outp->Storage), out);
				ANT_SERIALIZE_PROPERTY(Value, outp->Value, out);

				out << YAML::EndMap; // outp
			}
			out << YAML::EndSeq; // Outputs

			out << YAML::EndMap; // node
		}
		out << YAML::EndSeq; // Nodes
	}

	void DefaultGraphSerializer::SerializeLinks(YAML::Emitter& out, const std::vector<Link>& links)
	{
		out << YAML::Key << "Links" << YAML::Value;
		out << YAML::BeginSeq;
		for (const auto& link : links)
		{
			out << YAML::BeginMap; // link

			const auto& col = link.Color.Value;
			const glm::vec4 colOut(col.x, col.y, col.z, col.w);

			ANT_SERIALIZE_PROPERTY(ID, link.ID, out);
			ANT_SERIALIZE_PROPERTY(StartPinID, link.StartPinID, out);
			ANT_SERIALIZE_PROPERTY(EndPinID, link.EndPinID, out);
			ANT_SERIALIZE_PROPERTY(Colour, colOut, out);

			out << YAML::EndMap; // link
		}
		out << YAML::EndSeq; // Links
	}


	struct DefaultGraphSerializer::DeserializationFactory::PinCandidate : Pin
	{
		using Pin::Pin;

		std::string TypeString;	// implementation specific

		int GetType() const override { return -1; }
		std::string_view GetTypeString() const override { return TypeString; }
	};

	struct DefaultGraphSerializer::DeserializationFactory::NodeCandidate
	{
		UUID ID;
		std::string Category;
		std::string Name;
		NodeType Type;
		uint32_t NumInputs;
		uint32_t NumOutputs;
	};

	using PinCandidate = DefaultGraphSerializer::DeserializationFactory::PinCandidate;
	using NodeCandidate = DefaultGraphSerializer::DeserializationFactory::NodeCandidate;

	[[nodiscard]] std::optional<std::vector<PinCandidate>> TryLoadInputs(YAML::Node& inputs, const NodeCandidate& node)
	{
		if (!inputs.IsSequence())
			return {};

		std::vector<PinCandidate> list;

		uint32_t index = 0;
		for (auto& in : inputs)
		{
			UUID ID;
			std::string pinName;
			//std::string valueStr;
			choc::value::Value value;
			std::string pinType;
			std::string pinStorage;

			ANT_DESERIALIZE_PROPERTY(ID, ID, in, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(Name, pinName, in, std::string());
			ANT_DESERIALIZE_PROPERTY(Type, pinType, in, std::string());
			ANT_DESERIALIZE_PROPERTY(Storage, pinStorage, in, std::string());
			//ANT_DESERIALIZE_PROPERTY(Value, valueStr, in, std::string());
			ANT_DESERIALIZE_PROPERTY(Value, value, in, choc::value::Value());

			// TODO: load legacy saved valueStr, or manually rewrite YAML files for the new format?

			PinCandidate candidate;
			candidate.ID = ID;
			candidate.Name = pinName;
			candidate.Storage = StorageKindFromString(pinStorage);
			candidate.Value = value;
			candidate.Kind = PinKind::Input;
			candidate.TypeString = pinType;

			list.push_back(candidate);

#if 0
			bool isCustomValueType = choc::text::contains(valueStr, "Value");

			auto parseCustomValueType = [](const std::string& valueString) -> choc::value::Value
			{
				choc::value::Value value = choc::json::parse(valueString);

				if (value["TypeName"].isVoid())
				{
					ANT_CORE_ASSERT(false, "Failed to deserialize custom value type, missing \"TypeName\" property.");
					return {};
				}

				choc::value::Value customObject = choc::value::createObject(value["TypeName"].get<std::string>());
				if (value.isObject())
				{
					for (uint32_t i = 0; i < value.size(); i++)
					{
						choc::value::MemberNameAndValue nameValue = value.getObjectMemberAt(i);
						customObject.addMember(nameValue.name, nameValue.value);
					}
				}
				else
				{
					ANT_CORE_ASSERT(false, "Failed to load custom value type. It must be serialized as object.")
				}

				return customObject;
			};

			const EPinType type = PinTypeFromString(pinType);
			const StorageKind storage = StorageKindFromString(pinStorage);

			//? this can throw if trying tp parse super small (or weird?) float values like 9.999999974752428e-7
			choc::value::Value v = isCustomValueType ? parseCustomValueType(valueStr) : choc::json::parseValue(valueStr);

			// Chock loads int and float values from JSON string as int64 and double,
			// need to converst them back to what wee need
			if (!isCustomValueType)
			{
				if (storage != StorageKind::Array && !v.isString() && !v.isVoid()) // string can be name of connected Graph Input
				{
					switch (type)
					{
					case Ant::EPinType::Int: v = choc::value::createInt32(v.get<int32_t>()); break;
					case Ant::EPinType::Float: v = choc::value::createFloat32(v.get<float>()); break;
					default:
						break;
					}
				}
				else if (storage == StorageKind::Array && v.isArray() && v.size() && !v[0].isString())
				{
					for (auto& element : v)
					{
						switch (type)
						{
						case Ant::EPinType::Int: element = choc::value::createInt32(element.get<int32_t>()); break;
						case Ant::EPinType::Float: element = choc::value::createFloat32(element.get<float>()); break;
						default:
							break;
						}
					}
				}
			}
			newNode->Inputs.emplace_back(ID, pinName.c_str(),
				type,
				storage,
				v)
				->Kind = PinKind::Input;
#endif
		}

		return list;
	}

	[[nodiscard]] std::optional<std::vector<PinCandidate>> TryLoadOutputs(YAML::Node& outputs, const NodeCandidate& node)
	{
		if (!outputs.IsSequence())
			return {};

		std::vector<PinCandidate> list;

		uint32_t index = 0;
		for (auto& out : outputs)
		{
			UUID ID;
			std::string pinName;
			std::string valueStr;
			choc::value::Value value;
			std::string pinType;
			std::string pinStorage;

			ANT_DESERIALIZE_PROPERTY(ID, ID, out, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(Name, pinName, out, std::string());
			ANT_DESERIALIZE_PROPERTY(Type, pinType, out, std::string());
			ANT_DESERIALIZE_PROPERTY(Storage, pinStorage, out, std::string());
			//ANT_DESERIALIZE_PROPERTY(Value, valueStr, out, std::string());
			ANT_DESERIALIZE_PROPERTY(Value, value, out, choc::value::Value());

			PinCandidate candidate;
			candidate.ID = ID;
			candidate.Name = pinName;
			candidate.Storage = StorageKindFromString(pinStorage);
			candidate.Value = value;
			candidate.Kind = PinKind::Output;
			candidate.TypeString = pinType;

			list.push_back(candidate);

#if 0
			newNode->Outputs.emplace_back(ID, pinName.c_str(),
				PinTypeFromString(pinType),
				StorageKindFromString(pinStorage),
				choc::json::parseValue(valueStr)) //? what if the output is of the Object type?
				->Kind = PinKind::Output;
#endif
		}

		return list;
	}

	void DefaultGraphSerializer::TryLoadNodes(YAML::Node& data, std::vector<Node*>& nodes, const DeserializationFactory& factory)
	{
		// TODO: remove exceptions when we implement a dummy "invalid" node/pin types to display to the user?
		//		for now we just stop loading the whole graph if there's any error in the YAML document and/or node fabrication

		for (auto& node : data["Nodes"])
		{
			UUID nodeID;
			std::string nodeCategory;
			std::string nodeName;
			std::string location;
			std::string nodeTypeStr;
			glm::vec4 nodeColour;
			glm::vec2 nodeSize;

			ANT_DESERIALIZE_PROPERTY(ID, nodeID, node, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(Category, nodeCategory, node, std::string());
			ANT_DESERIALIZE_PROPERTY(Name, nodeName, node, std::string());
			ANT_DESERIALIZE_PROPERTY(Colour, nodeColour, node, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			ANT_DESERIALIZE_PROPERTY(Type, nodeTypeStr, node, std::string());	// TODO: YAML serialization for enums to/from string
			ANT_DESERIALIZE_PROPERTY(Size, nodeSize, node, glm::vec2());
			ANT_DESERIALIZE_PROPERTY(Location, location, node, std::string());

			const NodeType nodeType = NodeTypeFromString(nodeTypeStr);	// TODO: this could also be implmentation specific types

			NodeCandidate candidate;
			candidate.ID = nodeID;
			candidate.Category = nodeCategory;
			candidate.Name = nodeName;
			candidate.Type = nodeType;
			candidate.NumInputs = node["Inputs"] ? (uint32_t)node["Inputs"].size() : 0;
			candidate.NumOutputs = node["Outputs"] ? (uint32_t)node["Outputs"].size() : 0;


			std::optional<std::vector<PinCandidate>> candidateInputs;
			std::optional<std::vector<PinCandidate>> candidateOutputs;

			if (node["Inputs"])
			{
				if (!(candidateInputs = TryLoadInputs(node["Inputs"], candidate)))
				{
					// YAML file contains "Inputs" but we've failed to parse them
					throw std::runtime_error("Failed to load editor Node inputs '" + candidate.Name + "' inputs.");
				}
				else if (candidateInputs->size() != candidate.NumInputs)
				{
					// YAML file contains different number of entries for the Inputs than we've managed to deserialize
					throw std::runtime_error("Deserialized Node Inputs list doesn't match the number of serialized Node '" + candidate.Name + "' inputs.");
				}
			}

			if (node["Outputs"])
			{
				if (!(candidateOutputs = TryLoadOutputs(node["Outputs"], candidate)))
				{
					// YAML file contains "Outputs" but we've failed to parse them
					throw std::runtime_error("Failed to load editor Node '" + candidate.Name + "' outputs.");
				}
				else if (candidateOutputs->size() != candidate.NumOutputs)
				{
					// YAML file contains different number of entries for the Outputs than we've managed to deserialize
					throw std::runtime_error("Deserialized Node Outputs list doesn't match the number of serialized Node '" + candidate.Name + "' outputs.");
				}
			}

			// This is not going to load old Node configurations and enforce old to new Topology compatibility
			// TODO: we might want to still load old topology as an "invalid" dummy node to display it to the user
			Node* newNode = factory.ConstructNode(candidate, candidateInputs, candidateOutputs);

			if (!newNode)
			{
				throw std::runtime_error("Failed to construct deserialized Node '" + candidate.Name + "'.");
			}

			newNode->ID = candidate.ID;
			newNode->State = location;
			newNode->Color = ImColor(nodeColour.x, nodeColour.y, nodeColour.z, nodeColour.w);
			newNode->Size = ImVec2(nodeSize.x, nodeSize.y);

			if (newNode->Inputs.size() != candidate.NumInputs || newNode->Outputs.size() != candidate.NumOutputs)
			{
				delete newNode;

				// Factory Node might have changed and we've deserialized an old version of the Node
				throw std::runtime_error("Deserialized Node topology doesn't match factory Node '" + candidate.Name + "'.");
			}

			// Implementation specific construction and/or validation of Node Pins

			if (candidateInputs)
			{
				for (uint32_t i = 0; i < candidateInputs->size(); ++i)
				{
					const PinCandidate& candidatePin = candidateInputs->at(i);
					Pin* factoryPin = newNode->Inputs[i];

					if (!factory.DeserializePin(candidatePin, factoryPin, candidate))
					{
						delete newNode;
						// This error is pushed by the implementation
						throw std::runtime_error("Failed to deserialize/validate input Pin '" + candidatePin.Name + "' for a Node '" + candidate.Name + "'.");
					}

					factoryPin->ID = candidatePin.ID;
				}
			}

			if (candidateOutputs)
			{
				for (uint32_t i = 0; i < candidateOutputs->size(); ++i)
				{
					const PinCandidate& candidatePin = candidateOutputs->at(i);
					Pin* factoryPin = newNode->Outputs[i];

					if (!factory.DeserializePin(candidatePin, factoryPin, candidate))
					{
						delete newNode;
						// This error is pushed by the implementation
						throw std::runtime_error("Failed to deserialize/validate output Pin '" + candidatePin.Name + "' for a Node '" + candidate.Name + "'.");
					}

					factoryPin->ID = candidatePin.ID;
				}
			}

			nodes.push_back(newNode);
		}
	}

	void DefaultGraphSerializer::TryLoadLinks(YAML::Node& data, std::vector<Link>& links)
	{
		for (auto& link : data["Links"])
		{
			UUID ID;
			UUID StartPinID;
			UUID EndPinID;
			glm::vec4 colour;

			ANT_DESERIALIZE_PROPERTY(ID, ID, link, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(StartPinID, StartPinID, link, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(EndPinID, EndPinID, link, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(Colour, colour, link, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

			links.emplace_back(ID, StartPinID, EndPinID)
				.Color = ImColor(colour.x, colour.y, colour.z, colour.w);
		}
	}

	//! Old Node Graph serialization, kept as an example
#if 0
	std::string DefaultGraphSerializer::SerializeToYAML(Ref<SoundGraphAsset> soundGraphAsset) const
	{
		YAML::Emitter out;

		out << YAML::BeginMap; // Nodes, Links, Graph State

		//============================================================
		/// Nodes

		SerializeNodes(out, soundGraphAsset->Nodes);

#if 0
		out << YAML::Key << "Nodes" << YAML::Value;
		out << YAML::BeginSeq;
		for (auto& node : graph->Nodes)
		{
			out << YAML::BeginMap; // node

			const ImVec4& nodeCol = node.Color.Value;
			const ImVec2& nodeSize = node.Size;
			const glm::vec4 nodeColOut(nodeCol.x, nodeCol.y, nodeCol.z, nodeCol.w);
			const glm::vec2 nodeSizeOut(nodeSize.x, nodeSize.y);

			ANT_SERIALIZE_PROPERTY(ID, node.ID, out);
			ANT_SERIALIZE_PROPERTY(Category, node.Category, out);
			ANT_SERIALIZE_PROPERTY(Name, node.Name, out);
			ANT_SERIALIZE_PROPERTY(Colour, nodeColOut, out);
			ANT_SERIALIZE_PROPERTY(Type, NodeTypeToString(node.Type), out);
			ANT_SERIALIZE_PROPERTY(Size, nodeSizeOut, out);
			ANT_SERIALIZE_PROPERTY(Location, node.State, out);

			out << YAML::Key << "Inputs" << YAML::BeginSeq;
			for (auto& in : node.Inputs)
			{
				out << YAML::BeginMap; // in
				ANT_SERIALIZE_PROPERTY(ID, in.ID, out);
				ANT_SERIALIZE_PROPERTY(Name, in.Name, out);
				ANT_SERIALIZE_PROPERTY(Type, PinTypeToString(in.Type), out);
				ANT_SERIALIZE_PROPERTY(Storage, StorageKindToString(in.Storage), out);
				ANT_SERIALIZE_PROPERTY(Value, choc::json::toString(in.Value), out);
				out << YAML::EndMap; // in
			}
			out << YAML::EndSeq; // Inputs

			out << YAML::Key << "Outputs" << YAML::BeginSeq;
			for (auto& outp : node.Outputs)
			{
				out << YAML::BeginMap; // outp
				ANT_SERIALIZE_PROPERTY(ID, outp.ID, out);
				ANT_SERIALIZE_PROPERTY(Name, outp.Name, out);
				ANT_SERIALIZE_PROPERTY(Type, PinTypeToString(outp.Type), out);
				ANT_SERIALIZE_PROPERTY(Storage, StorageKindToString(outp.Storage), out);
				ANT_SERIALIZE_PROPERTY(Value, choc::json::toString(outp.Value), out);
				out << YAML::EndMap; // outp
			}
			out << YAML::EndSeq; // Outputs

			out << YAML::EndMap; // node
		}
		out << YAML::EndSeq; // Nodes
#endif

		//============================================================
		/// Links

		SerializeLinks(out, soundGraphAsset->Links);

#if 0
		out << YAML::Key << "Links" << YAML::Value;
		out << YAML::BeginSeq;
		for (auto& link : graph->Links)
		{
			out << YAML::BeginMap; // link

			const auto& col = link.Color.Value;
			const glm::vec4 colOut(col.x, col.y, col.z, col.w);

			ANT_SERIALIZE_PROPERTY(ID, link.ID, out);
			ANT_SERIALIZE_PROPERTY(StartPinID, link.StartPinID, out);
			ANT_SERIALIZE_PROPERTY(EndPinID, link.EndPinID, out);
			ANT_SERIALIZE_PROPERTY(Colour, colOut, out);

			out << YAML::EndMap; // link
		}
		out << YAML::EndSeq; // Links
#endif

		//============================================================
		/// Graph State

		ANT_SERIALIZE_PROPERTY(GraphState, soundGraphAsset->GraphState, out);

		out << YAML::EndMap; // Nodes, Links, Graph State
		return std::string(out.c_str());
		return {};
	}

	bool DefaultGraphSerializer::DeserializeFromYAML(const std::string& yamlString, Ref<SoundGraphAsset>& soundGraphAsset) const
	{
		YAML::Node data = YAML::Load(yamlString);

		soundGraphAsset = Ref<SoundGraphAsset>::Create();

		//============================================================
		/// Nodes

		TryLoadNodes(data, soundGraphAsset->Nodes);
#if 0
		for (auto& node : data["Nodes"])
		{
			UUID nodeID;
			std::string nodeCategory;
			std::string nodeName;
			std::string location;
			std::string nodeType;
			glm::vec4 nodeCol;
			glm::vec2 nodeS;

			ANT_DESERIALIZE_PROPERTY(ID, nodeID, node, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(Category, nodeCategory, node, std::string());
			ANT_DESERIALIZE_PROPERTY(Name, nodeName, node, std::string());
			ANT_DESERIALIZE_PROPERTY(Colour, nodeCol, node, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
			ANT_DESERIALIZE_PROPERTY(Type, nodeType, node, std::string());
			ANT_DESERIALIZE_PROPERTY(Size, nodeS, node, glm::vec2());
			ANT_DESERIALIZE_PROPERTY(Location, location, node, std::string());

			auto& newNode = graph->Nodes.emplace_back(nodeID, nodeName.c_str());
			newNode.Category = nodeCategory;
			newNode.State = location;
			newNode.Color = ImColor(nodeCol.x, nodeCol.y, nodeCol.z, nodeCol.w);
			newNode.Type = NodeTypeFromString(nodeType);
			newNode.Size = ImVec2(nodeS.x, nodeS.y);

			if (node["Inputs"])
			{
				for (auto& in : node["Inputs"])
				{
					UUID ID;
					std::string pinName;
					std::string valueStr;
					std::string pinType;
					std::string pinStorage;

					ANT_DESERIALIZE_PROPERTY(ID, ID, in, uint64_t(0));
					ANT_DESERIALIZE_PROPERTY(Name, pinName, in, std::string());
					ANT_DESERIALIZE_PROPERTY(Type, pinType, in, std::string());
					ANT_DESERIALIZE_PROPERTY(Storage, pinStorage, in, std::string());
					ANT_DESERIALIZE_PROPERTY(Value, valueStr, in, std::string());

					bool isCustomValueType = choc::text::contains(valueStr, "Value");

					auto parseCustomValueType = [](const std::string& valueString) -> choc::value::Value
					{
						choc::value::Value value = choc::json::parse(valueString);

						if (value["TypeName"].isVoid())
						{
							ANT_CORE_ASSERT(false, "Failed to deserialize custom value type, missing \"TypeName\" property.");
							return {};
						}

						choc::value::Value customObject = choc::value::createObject(value["TypeName"].get<std::string>());
						if (value.isObject())
						{
							for (uint32_t i = 0; i < value.size(); i++)
							{
								choc::value::MemberNameAndValue nameValue = value.getObjectMemberAt(i);
								customObject.addMember(nameValue.name, nameValue.value);
							}
						}
						else
						{
							ANT_CORE_ASSERT(false, "Failed to load custom value type. It must be serialized as object.")
						}

						return customObject;
					};

					newNode.Inputs.emplace_back(ID, pinName.c_str(),
						PinTypeFromString(pinType),
						StorageKindFromString(pinStorage),
						isCustomValueType ? parseCustomValueType(valueStr) : choc::json::parseValue(valueStr))
						.Kind = PinKind::Input;
				}
			}

			if (node["Outputs"])
			{
				for (auto& out : node["Outputs"])
				{
					UUID ID;
					std::string pinName;
					std::string valueStr;
					std::string pinType;
					std::string pinStorage;

					ANT_DESERIALIZE_PROPERTY(ID, ID, out, uint64_t(0));
					ANT_DESERIALIZE_PROPERTY(Name, pinName, out, std::string());
					ANT_DESERIALIZE_PROPERTY(Type, pinType, out, std::string());
					ANT_DESERIALIZE_PROPERTY(Storage, pinStorage, out, std::string());
					ANT_DESERIALIZE_PROPERTY(Value, valueStr, out, std::string());

					newNode.Outputs.emplace_back(ID, pinName.c_str(),
						PinTypeFromString(pinType),
						StorageKindFromString(pinStorage),
						choc::json::parseValue(valueStr))
						.Kind = PinKind::Output;
				}
			}
		}
#endif

		//============================================================
		/// Links

		TryLoadLinks(data, soundGraphAsset->Links);
#if 0
		for (auto& link : data["Links"])
		{
			UUID ID;
			UUID StartPinID;
			UUID EndPinID;
			glm::vec4 colour;

			ANT_DESERIALIZE_PROPERTY(ID, ID, link, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(StartPinID, StartPinID, link, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(EndPinID, EndPinID, link, uint64_t(0));
			ANT_DESERIALIZE_PROPERTY(Colour, colour, link, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

			graph->Links.emplace_back(ID, StartPinID, EndPinID)
				.Color = ImColor(colour.x, colour.y, colour.z, colour.w);
		}
#endif

		//============================================================
		/// Graph State

		ANT_DESERIALIZE_PROPERTY(GraphState, soundGraphAsset->GraphState, data, std::string());
		return true;
	}
#endif

	//============================================================
	/// SoundGraph Graph Serializer

	SoundGraphGraphSerializer::SoundGraphGraphSerializer()
	{
		m_Cache = CreateScope<SoundGraphCache>(MAX_NUM_CACHED_GRAPHS);
	}

	SoundGraphGraphSerializer::~SoundGraphGraphSerializer() = default;

	void SoundGraphGraphSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<SoundGraphAsset> graph = asset.As<SoundGraphAsset>();

		// Out
		std::ofstream fout(Project::GetEditorAssetManager()->GetFileSystemPath(metadata));
		ANT_CORE_VERIFY(fout.good());
		std::string yamlString = SerializeToYAML(graph);
		fout << yamlString;
	}

	bool SoundGraphGraphSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		const auto filepath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata);
		std::ifstream stream(filepath);
		if (!stream.is_open())
		{
			ANT_CONSOLE_LOG_ERROR("Failed to open SoundGraph asset file to deserialize: {}", filepath.string());
			return false;
		}

		std::stringstream strStream;
		strStream << stream.rdbuf();

		Ref<SoundGraphAsset> soundGraph;
		if (!DeserializeFromYAML(strStream.str(), soundGraph))
		{
			ANT_CONSOLE_LOG_ERROR("Failed to deserialize SoundGraph: {}", filepath.string());
			return false;
		}

		if (!soundGraph->CachedPrototype.empty() && !soundGraph->Prototype)
		{
			// TODO: need to set new cache directory when new project is loaded
			const std::filesystem::path cacheDirectory = Project::GetCacheDirectory() / "SoundGraph";
			m_Cache->SetCacheDirectory(cacheDirectory);

			const std::string nameOld = metadata.FilePath.stem().string();
			// Cherno Hack
			std::string name = soundGraph->CachedPrototype.stem().string();
			if (name.find("sound_graph_cache_") != std::string::npos)
				name = name.substr(18);

			ANT_CORE_VERIFY(nameOld == name);
			soundGraph->Prototype = m_Cache->ReadPtototype(name.c_str());
		}

		//? Temprorary hack. If we've compiled graph in Editor, we want to keep Prototyp alive to play in editor
		// this is for just compiled assets
		if (asset && !soundGraph->Prototype)
		{
			if (Ref<SoundGraph::Prototype> prototype = asset.As<SoundGraphAsset>()->Prototype)
				soundGraph->Prototype = prototype;
		}

		asset = soundGraph;
		asset->Handle = metadata.Handle;
		return true;
	}

	bool SoundGraphGraphSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		Ref<SoundGraphAsset> soundGraph = AssetManager::GetAsset<SoundGraphAsset>(handle);

		std::string yamlString = SerializeToYAML(soundGraph);
		outInfo.Offset = stream.GetStreamPosition();
		stream.WriteString(yamlString);	// TODO: Why do we serialize YAML string to asset pack? In Runtime we should only use Graph Prototypes.
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		return true;
	}

	Ref<Asset> SoundGraphGraphSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		stream.SetStreamPosition(assetInfo.PackedOffset);
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<SoundGraphAsset> soundGraph;
		bool result = DeserializeFromYAML(yamlString, soundGraph);
		if (!result)
			return nullptr;

		if (!soundGraph->CachedPrototype.empty() && !soundGraph->Prototype)
		{
			// TODO: need to set new cache directory when new project is loaded
			const std::filesystem::path cacheDirectory = Project::GetCacheDirectory() / "SoundGraph";
			m_Cache->SetCacheDirectory(cacheDirectory);

			const std::string name = soundGraph->CachedPrototype.stem().string();
			soundGraph->Prototype = m_Cache->ReadPtototype(name.c_str());
		}

		//? Temprorary hack. If we've compiled graph in Editor, we want to keep Prototyp alive to play in editor
		// this is for just compiled assets
		if (soundGraph && !soundGraph->Prototype)
		{
			if (Ref<SoundGraph::Prototype> prototype = soundGraph->Prototype)
				soundGraph->Prototype = prototype;
		}

		return soundGraph;
	}

	std::string SoundGraphGraphSerializer::SerializeToYAML(Ref<SoundGraphAsset> graph) const
	{
		YAML::Emitter out;

		out << YAML::BeginMap; // Nodes, Links, Graph State

		// Nodes
		DefaultGraphSerializer::SerializeNodes(out, graph->Nodes);

		// Links
		DefaultGraphSerializer::SerializeLinks(out, graph->Links);

		// Graph IO
		ANT_SERIALIZE_PROPERTY(GraphInputs, graph->GraphInputs.ToExternalValue(), out);
		ANT_SERIALIZE_PROPERTY(GraphOutputs, graph->GraphOutputs.ToExternalValue(), out);
		ANT_SERIALIZE_PROPERTY(LocalVariables, graph->LocalVariables.ToExternalValue(), out);

		// Graph State
		ANT_SERIALIZE_PROPERTY(GraphState, graph->GraphState, out);

		// Wave Sources
		out << YAML::Key << "Waves" << YAML::Value;
		out << YAML::BeginSeq;
		for (const auto& wave : graph->WaveSources)
		{
			out << YAML::BeginMap; // wave
			{
				ANT_SERIALIZE_PROPERTY(WaveAssetHandle, wave, out);
			}
			out << YAML::EndMap; // wave
		}
		out << YAML::EndSeq; // Waves

		//? should probably serialize it as binary
		// Graph Prototype
#if 0
		if (graph->Prototype)
		{
			out << YAML::Key << "Prototype" << YAML::Value;
			out << YAML::BeginMap; // Prototype

			const auto& prototype = graph->Prototype;

			ANT_SERIALIZE_PROPERTY(PrototypeName, prototype->DebugName, out);
			ANT_SERIALIZE_PROPERTY(PrototypeID, prototype->ID, out);

			auto serializeEndpoint = [&out](const SoundGraph::Prototype::Endpoint& endpoint)
			{
				out << YAML::BeginMap; // endpoing
				{
					ANT_SERIALIZE_PROPERTY(EndpointID, (uint32_t)endpoint.EndpointID, out);
					ANT_SERIALIZE_PROPERTY(DefaultValue, choc::json::toString(endpoint.DefaultValue), out); // TODO: need to store type
				}
				out << YAML::EndMap; // endpoint
			};

			auto serializeSequence = [&out](const char* sequenceName, auto serializationFunction)
			{
				out << YAML::Key << sequenceName << YAML::Value;
				out << YAML::BeginSeq;
				{
					serializationFunction();
				}
				out << YAML::EndSeq; // Inputs
			};

			serializeSequence("Inputs", [&]
				{
					for (const auto& in : prototype->Inputs)
						serializeEndpoint(in);
				});

			serializeSequence("Outputs", [&]
				{
					for (auto& out : prototype->Outputs)
					{
						serializeEndpoint(out);
					}
				});

			serializeSequence("OutputChannelIDs", [&]
				{
					for (const auto& id : prototype->OutputChannelIDs)
					{
						out << YAML::BeginMap;
						ANT_SERIALIZE_PROPERTY(ID, (uint32_t)id, out);
						out << YAML::EndMap;
					}
				});

			serializeSequence("Nodes", [&]
				{
					for (const auto& node : prototype->Nodes)
					{
						out << YAML::BeginMap;
						ANT_SERIALIZE_PROPERTY(NodeTypeID, (uint32_t)node.NodeTypeID, out);
						ANT_SERIALIZE_PROPERTY(ID, (uint64_t)node.ID, out);

						serializeSequence("DefaultValuePlugs", [&]
							{
								for (const auto& valuePlug : node.DefaultValuePlugs)
								{
									serializeEndpoint(valuePlug);
								}
							});
						out << YAML::EndMap;
					}
				});

			serializeSequence("Connections", [&]
				{
					for (const auto& connection : prototype->Connections)
					{
						out << YAML::BeginMap; // connection
						{
							out << YAML::Key << "Source" << YAML::Value;
							out << YAML::BeginMap; // endpoing
							{
								ANT_SERIALIZE_PROPERTY(NodeID, (uint64_t)connection.Source.NodeID, out);
								ANT_SERIALIZE_PROPERTY(EndpointID, (uint32_t)connection.Source.EndpointID, out);
							}
							out << YAML::EndMap; // endpoint

							out << YAML::Key << "Destination" << YAML::Value;
							out << YAML::BeginMap; // endpoing
							{
								ANT_SERIALIZE_PROPERTY(NodeID, (uint64_t)connection.Destination.NodeID, out);
								ANT_SERIALIZE_PROPERTY(EndpointID, (uint32_t)connection.Destination.EndpointID, out);
							}
							out << YAML::EndMap; // endpoint

							ANT_SERIALIZE_PROPERTY(Type, (uint32_t)connection.Type, out);
						}
						out << YAML::EndMap; // connection
					}
				});

			out << YAML::EndMap; // Prototype
		}
#endif
		const std::string pathStr = graph->CachedPrototype.empty() ? std::string("") : graph->CachedPrototype.string();
		ANT_SERIALIZE_PROPERTY(CachedPrototype, pathStr, out);

		out << YAML::EndMap; // Nodes, Links, Graph State, Waves, Prototype

		return std::string(out.c_str());
	}

	bool SoundGraphGraphSerializer::DeserializeFromYAML(const std::string& yamlString, Ref<SoundGraphAsset>& graph) const
	{
		YAML::Node data;

		try
		{
			data = YAML::Load(yamlString);
		}
		catch (const std::exception& e)
		{
			ANT_CONSOLE_LOG_FATAL(e.what());
			return false;
		}

		graph = Ref<SoundGraphAsset>::Create();

		// Graph IO (must be deserialize before nodes to be able to spawn IO nodes)
		{
			choc::value::Value graphInputsValue;
			choc::value::Value graphOutputsValue;
			choc::value::Value graphLocalVariablesValue;

			ANT_DESERIALIZE_PROPERTY(GraphInputs, graphInputsValue, data, choc::value::Value());
			ANT_DESERIALIZE_PROPERTY(GraphOutputs, graphOutputsValue, data, choc::value::Value());
			ANT_DESERIALIZE_PROPERTY(LocalVariables, graphLocalVariablesValue, data, choc::value::Value());

			graph->GraphInputs = Utils::PropertySet::FromExternalValue(graphInputsValue);
			graph->GraphOutputs = Utils::PropertySet::FromExternalValue(graphOutputsValue);
			graph->LocalVariables = Utils::PropertySet::FromExternalValue(graphLocalVariablesValue);
		}

		// Nodes

		auto constructNode = [&graph](const NodeCandidate& candidate, const std::optional<std::vector<PinCandidate>>& inputs, const std::optional<std::vector<PinCandidate>>& outputs)
		{
			// graph IO, Local Variables
			auto optType = magic_enum::enum_cast<ESoundGraphPropertyType>(choc::text::replace(candidate.Category, " ", ""));

			Node* newNode = nullptr;

			if (optType.has_value())
			{
				auto getLocalVariableName = [&]()
				{
					ANT_CORE_ASSERT(inputs.has_value() || outputs.has_value())

						std::string name;

					if (outputs && !outputs->empty())		name = outputs->at(0).Name;
					else if (inputs && !inputs->empty())	name = inputs->at(0).Name;

					ANT_CORE_ASSERT(!name.empty());

					return name;
				};

				switch (*optType)
				{
				case ESoundGraphPropertyType::Input: newNode = Nodes::SoundGraphNodeFactory::SpawnGraphPropertyNode(graph, getLocalVariableName(), *optType); break;
				case ESoundGraphPropertyType::Output: newNode = Nodes::SoundGraphNodeFactory::SpawnGraphPropertyNode(graph, getLocalVariableName(), *optType); break;
				case ESoundGraphPropertyType::LocalVariable: newNode = Nodes::SoundGraphNodeFactory::SpawnGraphPropertyNode(graph, candidate.Name, *optType, (bool)candidate.NumOutputs); break;
				default:
					ANT_CORE_ASSERT(false, "Deserialized invalid SoundGraph property type.");
					break;
				}
			}
			else
			{
				if (candidate.Type == NodeType::Comment)
				{
					// Cooments are special case where the name is editable and is displayed in the comment header
					//! Alternatively we could make a special pin to use as the header for the comment node, just to keep this logic more generic.
					newNode = Nodes::SoundGraphNodeFactory::SpawnNodeStatic(candidate.Category, magic_enum::enum_name<NodeType::Comment>());
					newNode->Name = candidate.Name;
				}
				else
				{
					newNode = Nodes::SoundGraphNodeFactory::SpawnNodeStatic(candidate.Category, candidate.Name);
				}
			}

			ANT_CORE_ASSERT(!newNode ^ (newNode->Type == candidate.Type));

			return newNode;
		};

		auto deserializePin = [](const PinCandidate& candidate, Pin* factoryPin, const NodeCandidate& node)
		{
			auto optType = magic_enum::enum_cast<Nodes::SGTypes::ESGPinType>(candidate.GetTypeString());

			if (!optType.has_value())
				return false;

			// Check the type is correct
			const Nodes::SGTypes::ESGPinType candidateType = *optType;

			if (candidateType != factoryPin->GetType())
			{
				const std::string_view candidateTypeStr = magic_enum::enum_name<Nodes::SGTypes::ESGPinType>(candidateType);
				const std::string_view factoryPinTypeStr = factoryPin->GetTypeString();

				ANT_CONSOLE_LOG_ERROR("Pin type of the deserialized Node Pin ({0} - '{1}' ({2})) does not match the Pin type of the factory Node Pin ({0} - '{3}' ({4})).",
					node.Name, candidate.Name, candidateTypeStr, factoryPin->Name, factoryPinTypeStr);
				return false;
			}
			else if (candidate.Name != factoryPin->Name)
			{
				// TODO: this may not be correct when/if we change the Comment node to use Pin to display text instead of Node name
				ANT_CONSOLE_LOG_ERROR("Pin name of the deserialized Node Pin ({0} - '{1}') does not match the Pin name of the factory Node Pin ({0} - '{2}').", node.Name, candidate.Name, factoryPin->Name);
				return false;
			}

			// Flow or Audio types are not serialized
			if (factoryPin->IsType(Nodes::SGTypes::Flow) || factoryPin->IsType(Nodes::SGTypes::Audio))
				return true;

			//? (Old JSON serialization) Deserialize value from string
#if 0
			const bool isLegacyCustomValueType = choc::text::contains(valueStr, "Value");
			const bool isCustomValueType = !isLegacyCustomValueType && choc::text::contains(valueStr, "Data");
			const bool isBasicChocValue = !isLegacyCustomValueType && !isCustomValueType;

			auto parseBasicValue = [&pin, type](std::string_view valueString) -> choc::value::Value
			{
				choc::value::Value value = choc::json::parseValue(valueString);

				// choc json arithmetic value type is just 'number' (double),
				// we need to cast it to the correct type

				if (pin->Storage == StorageKind::Value && !value.isString() && !value.isVoid()) // string can be name of connected Graph Input
				{
					switch (type)
					{
					case Nodes::SGTypes::ESGPinType::Int: value = choc::value::createInt32(value.get<int32_t>()); break;
					case Nodes::SGTypes::ESGPinType::Float: value = choc::value::createFloat32(value.get<float>()); break;
					default:
						break;
					}
				}
				else if (pin->Storage == StorageKind::Array && value.isArray() && value.size() && !value[0].isString())
				{
					for (auto& element : value)
					{
						switch (type)
						{
						case Nodes::SGTypes::ESGPinType::Int: element = choc::value::createInt32(element.get<int32_t>()); break;
						case Nodes::SGTypes::ESGPinType::Float: element = choc::value::createFloat32(element.get<float>()); break;
						default:
							break;
						}
					}
				}

				return value;
			};

			auto parseCustomValueObject = [&pin](std::string_view valueString) -> choc::value::Value
			{
				const choc::value::Value valueObj = choc::json::parseValue(valueString);

				// TODO: choc value to json overwrites class name with "JSON", instead of json, parse Value object to/from YAML

				if (valueObj.getType() == pin->Value.getType())
				{
					return valueObj;
				}
				else
				{
					// This can trigger if we've trying to load old version of a Node.
					ANT_CORE_ASSERT(false, "Type of the deserialized Node Pin does not match the type of the factory Node Pin.");

					return choc::value::Value();
				}
			};

			auto parseLegacyCustomValueType = [](std::string_view valueString) -> choc::value::Value
			{
				choc::value::Value value = choc::json::parse(valueString);

				if (value["TypeName"].isVoid())
				{
					ANT_CORE_ASSERT(false, "Failed to deserialize custom value type, missing \"TypeName\" property.");
					return {};
				}

				choc::value::Value customObject = choc::value::createObject(value["TypeName"].get<std::string>());
				if (value.isObject())
				{
					for (uint32_t i = 0; i < value.size(); i++)
					{
						choc::value::MemberNameAndValue nameValue = value.getObjectMemberAt(i);
						customObject.addMember(nameValue.name, nameValue.value);
					}
				}
				else
				{
					ANT_CORE_ASSERT(false, "Failed to load custom value type. It must be serialized as object.")
				}

				return customObject;
			};

			const choc::value::Value deserializedValue = isBasicChocValue ? parseBasicValue(valueStr)
				: isCustomValueType ? parseCustomValueObject(valueStr)
				: parseLegacyCustomValueType(valueStr);
#endif

			// TODO: for now we use 'void' and 'string' for connected pins (this is needed for graph parsing,
			//		but perhaps we could assign 'void' type for the compilation step only?)
			if (candidate.Value.getType() == factoryPin->Value.getType() || candidate.Value.isVoid() || candidate.Value.isString())
			{
				factoryPin->Value = candidate.Value;
				return true;
			}
			else
			{
				// This can trigger if we've trying to load old version of a Node.
				ANT_CONSOLE_LOG_ERROR("Value type of the deserialized Node Pin ({0} - '{1}') does not match the Value type of the factory Node Pin ({0} - '{2}').", node.Name, candidate.Name, factoryPin->Name);
				return false;
			}
		};

		DefaultGraphSerializer::DeserializationFactory factory{ constructNode, deserializePin };

		try
		{
			DefaultGraphSerializer::TryLoadNodes(data, graph->Nodes, factory);
		}
		catch (const std::exception& e)
		{
			ANT_CONSOLE_LOG_FATAL(e.what());

			graph = Ref<SoundGraphAsset>::Create();
			return false;
		}

		// Assign enum tokens to pins
		for (auto& node : graph->Nodes)
		{
			for (auto& pin : node->Inputs)
			{
				if (pin->IsType(Nodes::SGTypes::ESGPinType::Enum))
				{
					if (const auto* tokens = Nodes::SoundGraphNodeFactory::GetEnumTokens(node->Name, pin->Name))
						pin->EnumTokens = tokens;
				}
			}
		}

		// Links
		DefaultGraphSerializer::TryLoadLinks(data, graph->Links);

		// Graph State
		ANT_DESERIALIZE_PROPERTY(GraphState, graph->GraphState, data, std::string());

		// Wave Sources
		for (auto& wave : data["Waves"])
		{
			UUID ID;

			ANT_DESERIALIZE_PROPERTY(WaveAssetHandle, ID, wave, uint64_t(0));
			graph->WaveSources.push_back(ID);
		}

		// Graph Prototype
		ANT_DESERIALIZE_PROPERTY(CachedPrototype, graph->CachedPrototype, data, std::string());
		return true;
	}
}