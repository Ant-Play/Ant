#pragma once

#include "Ant/Asset/Asset.h"

#include "Ant/Editor/NodeGraphEditor/PropertySet.h"
#include "Nodes.h"
#include "Ant/Audio/SoundGraph/SoundGraph.h"
#include "Ant/Audio/SoundGraph/SoundGraphPrototype.h"

#include <filesystem>

namespace Ant
{
	enum ESoundGraphPropertyType : uint8_t
	{
		Invalid = 0, Input, Output, LocalVariable
	};

	// TODO: technically only the Prototype is needed for the runtime from SoundGraphAsset
	class SoundGraphAsset : public Asset
	{
	public:
		std::vector<Node*> Nodes;
		std::vector<Link> Links;
		std::string GraphState; //? Should not have this in Runtime asset

		Utils::PropertySet GraphInputs;
		Utils::PropertySet GraphOutputs;
		Utils::PropertySet LocalVariables;

		Ref<SoundGraph::Prototype> Prototype; // TODO: add serialization / caching
		std::filesystem::path CachedPrototype;

		std::vector<UUID> WaveSources;

		SoundGraphAsset() = default;
		virtual ~SoundGraphAsset()
		{
			for (auto* node : Nodes)
				delete node;
		}

		static AssetType GetStaticType() { return AssetType::SoundGraphSound; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }
	};

} // namespace Ant