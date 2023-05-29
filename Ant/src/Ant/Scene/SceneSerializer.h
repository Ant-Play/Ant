#pragma once
#include "Scene.h"

#include "Ant/Serialization/FileStream.h"
#include "Ant/Asset/AssetSerializer.h"

namespace YAML {
	class Emitter;
	class Node;
}

namespace Ant{

	class SceneSerializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::filesystem::path& filepath);
		void SerializeToYAML(YAML::Emitter& out);
		bool DeserializeFromYAML(const std::string& yamlString);
		void SerializeRuntime(const std::filesystem::path& filepath);

		bool Deserialize(const std::filesystem::path& filepath);
		bool DeserializeRuntime(const std::filesystem::path& filepath);

		bool SerializeToAssetPack(FileStreamWriter& stream, AssetSerializationInfo& outInfo);
		bool DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::SceneInfo& sceneInfo);

		bool DeserializeReferencedPrefabs(const std::filesystem::path& filepath, std::unordered_set<AssetHandle>& outPrefabs);
	public:
		static void SerializeEntity(YAML::Emitter& out, Entity entity);
		static void DeserializeEntities(YAML::Node& entitiesNode, Ref<Scene> scene);
	public:
		inline static std::string_view FileFilter = "Ant Scene (*.ascene)\0*.ascene\0";
		inline static std::string_view DefaultExtension = ".ascene";
	private:
		Ref<Scene> m_Scene;
	};
}