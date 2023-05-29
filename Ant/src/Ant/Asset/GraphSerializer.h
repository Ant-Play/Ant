#pragma once

#include "AssetSerializer.h"
#include "Ant/Serialization/FileStream.h"
#include "Ant/Editor/NodeGraphEditor/Nodes.h"
#include "Ant/Editor/NodeGraphEditor/NodeGraphAsset.h"
#include "yaml-cpp/yaml.h"

namespace Ant{

	class SoundGraphCache;

	//==================================================================================
	/*	Graph serializer base provides static utilities for implementation
		to serialize and deserialize graph data.

		Implementation can deserialize nodes on its own, or use handy static
		TryLoadNodes() utility, which first deserializes the information about potential
		Node and Pins and then passes this info to the implementation factory callbacks
		to handle concrete construction and validation.
	*/

	class DefaultGraphSerializer : public AssetSerializer
	{
	public:
		static void SerializeNodes(YAML::Emitter& out, const std::vector<Node*>& nodes);
		static void SerializeLinks(YAML::Emitter& out, const std::vector<Link>& links);

		//==================================================================================
		/*	Implementation must provide this factory to deserialize nodes with static
			TryLoadNodes() function.

			(Note: if an implementation of GraphSerializer is in a different
			.cpp, definitions of the NodeCandidate and PinCandidate can be moved
			out of GraphSerializer.cpp into this header to make them visible)
		*/

		struct DeserializationFactory
		{
			struct NodeCandidate;	// Deserialized info about a Node, may or may not be valid
			struct PinCandidate;	// Deserialized info about a Pin of a Node, may or may not be valid

			/*	This factory function should construct a Node with default Input and Output pins
				as well as assign default and deserialized values from the candidate if required.
			*/
			std::function<Node* (const NodeCandidate& candidate,
				const std::optional<std::vector<PinCandidate>>& inputs,
				const std::optional<std::vector<PinCandidate>>& outputs)> ConstructNode;

			/*	This factory function should deserialize values from the candidate to the previously
				constructed default factory pin as well as do any required validation.
			*/
			std::function<bool(const PinCandidate& candidate,
				Pin* factoryPin,
				const NodeCandidate& nodeCandidate)> DeserializePin;
		};

		/*	Try to load graph Nodes from YAML. This function parses YAML into NodeCandidates and PinCandidates,
			which are then passed to implementation provided factory to deserialize and validate.

			This function throws an exception if deserialization fails, the caller must handle it!
		*/
		static void TryLoadNodes(YAML::Node& in, std::vector<Node*>& nodes, const DeserializationFactory& factory);
		static void TryLoadLinks(YAML::Node& in, std::vector<Link>& links);
	private:
	};

	class SoundGraphGraphSerializer : public AssetSerializer
	{
	public:
		SoundGraphGraphSerializer();
		~SoundGraphGraphSerializer();

		void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
		bool TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const override;

		bool SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const override;
		Ref<Asset> DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const override;
	private:
		std::string SerializeToYAML(Ref<SoundGraphAsset> soundGraphAsset) const;
		bool DeserializeFromYAML(const std::string& yamlString, Ref<SoundGraphAsset>& soundGraphAsset) const;
	private:
		Scope<SoundGraphCache> m_Cache;
	};

}