#pragma once

#include <unordered_map>

#include "AssetTypes.h"

namespace Ant{

	inline static std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
	{
		// Ant types
		{ ".ascene", AssetType::Scene },
		{ ".amesh", AssetType::Mesh },
		{ ".asmesh", AssetType::StaticMesh },
		{ ".amaterial", AssetType::Material },
		{ ".askel", AssetType::Skeleton },
		{ ".aanim", AssetType::Animation },
		{ ".aanimc", AssetType::AnimationController },
		{ ".aprefab", AssetType::Prefab },
		{ ".asoundc", AssetType::SoundConfig },
		{ ".apm", AssetType::PhysicsMat },

		{ ".cs", AssetType::ScriptFile },

		// mesh/animation source
		{ ".fbx", AssetType::MeshSource },
		{ ".gltf", AssetType::MeshSource },
		{ ".glb", AssetType::MeshSource },
		{ ".obj", AssetType::MeshSource },

		// Textures
		{ ".png", AssetType::Texture },
		{ ".jpg", AssetType::Texture },
		{ ".jpeg", AssetType::Texture },
		{ ".hdr", AssetType::EnvMap },

		// Audio
		{ ".wav", AssetType::Audio },
		{ ".ogg", AssetType::Audio },

		// Fonts
		{ ".ttf", AssetType::Font },
		{ ".ttc", AssetType::Font },
		{ ".otf", AssetType::Font },

		// Mesh Collider
		{ ".amc", AssetType::MeshCollider },

		// Graphs
		{ ".sound_graph", AssetType::SoundGraphSound }
	};
}