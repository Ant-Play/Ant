#include "antpch.h"
#include "AnimationAssetSerializer.h"

#include "Animation.h"
#include "AnimationController.h"
#include "Skeleton.h"

#include "Ant/Asset/AssetManager.h"

#include "Ant/Renderer/Mesh.h"

#include "Ant/Utilities/YAMLSerializationHelpers.h"

#include "yaml-cpp/yaml.h"

namespace Ant{

	//////////////////////////////////////////////////////////////////////////////////
	// SkeletonAssetSerializer
	//////////////////////////////////////////////////////////////////////////////////

	

	void SkeletonAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<SkeletonAsset> skeleton = asset.As<SkeletonAsset>();

		std::ofstream fout(Project::GetEditorAssetManager()->GetFileSystemPath(metadata));
		ANT_CORE_VERIFY(fout.good());
		std::string yamlString = SerializeToYAML(skeleton);
		fout << yamlString;
	}

	bool SkeletonAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		auto filepath = Project::GetAssetDirectory() / metadata.FilePath;
		std::ifstream stream(filepath);
		ANT_CORE_ASSERT(stream);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		Ref<SkeletonAsset> skeleton;
		bool result = DeserializeFromYAML(strStream.str(), skeleton);
		if (!result)
			return false;

		asset = skeleton;
		asset->Handle = metadata.Handle;
		return true;
	}

	bool SkeletonAssetSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		Ref<SkeletonAsset> skeleton = AssetManager::GetAsset<SkeletonAsset>(handle);
		std::string yamlString = SerializeToYAML(skeleton);

		outInfo.Offset = stream.GetStreamPosition();
		stream.WriteString(yamlString);
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		return true;
	}

	Ref<Asset> SkeletonAssetSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		stream.SetStreamPosition(assetInfo.PackedOffset);
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<SkeletonAsset> skeletonAsset;
		bool result = DeserializeFromYAML(yamlString, skeletonAsset);
		if (!result)
			return nullptr;

		return skeletonAsset;
	}

	std::string SkeletonAssetSerializer::SerializeToYAML(Ref<SkeletonAsset> skeleton) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Skeleton" << YAML::Value;
			out << YAML::BeginMap;
			{
				out << YAML::Key << "MeshSource" << YAML::Value << skeleton->GetMeshSource()->Handle;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		return std::string(out.c_str());
	}

	bool SkeletonAssetSerializer::DeserializeFromYAML(const std::string& yamlString, Ref<SkeletonAsset>& skeleton) const
	{
		YAML::Node data = YAML::Load(yamlString);
		if (!data["Skeleton"])
			return false;

		YAML::Node rootNode = data["Skeleton"];
		if (!rootNode["MeshSource"])
			return false;

		AssetHandle meshSourceHandle = 0;
		meshSourceHandle = rootNode["MeshSource"].as<uint64_t>(meshSourceHandle);

		if (!AssetManager::IsAssetHandleValid(meshSourceHandle))
			return false;

		skeleton = Ref<SkeletonAsset>::Create(meshSourceHandle);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// AnimationAssetSerializer
	//////////////////////////////////////////////////////////////////////////////////

	void AnimationAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<AnimationAsset> animationAsset = asset.As<AnimationAsset>();

		std::ofstream fout(Project::GetEditorAssetManager()->GetFileSystemPath(metadata));
		ANT_CORE_VERIFY(fout.good());
		std::string yamlString = SerializeToYAML(animationAsset);
		fout << yamlString;
	}

	bool AnimationAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		auto filepath = Project::GetAssetDirectory() / metadata.FilePath;
		std::ifstream stream(filepath);
		ANT_CORE_ASSERT(stream);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		Ref<AnimationAsset> animationAsset;
		bool result = DeserializeFromYAML(strStream.str(), animationAsset);
		if (!result)
			return false;

		asset = animationAsset;
		asset->Handle = metadata.Handle;
		return true;
	}

	bool AnimationAssetSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		Ref<AnimationAsset> animationAsset = AssetManager::GetAsset<AnimationAsset>(handle);
		std::string yamlString = SerializeToYAML(animationAsset);

		outInfo.Offset = stream.GetStreamPosition();
		stream.WriteString(yamlString);
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		return true;
	}

	Ref<Asset> AnimationAssetSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		stream.SetStreamPosition(assetInfo.PackedOffset);
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<AnimationAsset> animationAsset;
		bool result = DeserializeFromYAML(yamlString, animationAsset);
		if (!result)
			return nullptr;

		return animationAsset;
	}

	std::string AnimationAssetSerializer::SerializeToYAML(Ref<AnimationAsset> animationAsset) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "Animation" << YAML::Value;
			out << YAML::BeginMap;
			{
				out << YAML::Key << "MeshSource" << YAML::Value << animationAsset->GetMeshSource()->Handle;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		return std::string(out.c_str());
	}

	bool AnimationAssetSerializer::DeserializeFromYAML(const std::string& yamlString, Ref<AnimationAsset>& animationAsset) const
	{
		YAML::Node data = YAML::Load(yamlString);
		if (!data["Animation"])
			return false;

		YAML::Node rootNode = data["Animation"];
		if (!rootNode["MeshSource"])
			return false;

		AssetHandle meshSourceHandle = 0;
		meshSourceHandle = rootNode["MeshSource"].as<uint64_t>(meshSourceHandle);

		if (!AssetManager::IsAssetHandleValid(meshSourceHandle))
			return false;

		animationAsset = Ref<AnimationAsset>::Create(meshSourceHandle);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// AnimationControllerAssetSerializer
	//////////////////////////////////////////////////////////////////////////////////

	void AnimationControllerAssetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		// Note: This is likely to be entirely replaced by "node-graph" based serialization.
		//       So don't take anything in here too seriously.

		Ref<AnimationController> animationController = asset.As<AnimationController>();

		std::ofstream fout(Project::GetEditorAssetManager()->GetFileSystemPath(metadata));
		ANT_CORE_VERIFY(fout.good());

		std::string yamlString = SerializeToYAML(animationController);
		fout << yamlString;
	}

	bool AnimationControllerAssetSerializer::TryLoadData(const AssetMetadata& metadata, Ref<Asset>& asset) const
	{
		std::ifstream stream(Project::GetEditorAssetManager()->GetFileSystemPath(metadata));
		if (!stream.is_open())
			return false;

		std::stringstream strStream;
		strStream << stream.rdbuf();

		Ref<AnimationController> animationController;
		if (DeserializeFromYAML(strStream.str(), animationController))
		{
			asset = animationController;
			asset->Handle = metadata.Handle;
			return true;  // We have loaded something.  It could be invalid.
		}
		return false;
	}

	bool AnimationControllerAssetSerializer::SerializeToAssetPack(AssetHandle handle, FileStreamWriter& stream, AssetSerializationInfo& outInfo) const
	{
		Ref<AnimationController> animationController = AssetManager::GetAsset<AnimationController>(handle);
		std::string yamlString = SerializeToYAML(animationController);

		outInfo.Offset = stream.GetStreamPosition();
		stream.WriteString(yamlString);
		outInfo.Size = stream.GetStreamPosition() - outInfo.Offset;
		return true;
	}

	Ref<Asset> AnimationControllerAssetSerializer::DeserializeFromAssetPack(FileStreamReader& stream, const AssetPackFile::AssetInfo& assetInfo) const
	{
		stream.SetStreamPosition(assetInfo.PackedOffset);
		std::string yamlString;
		stream.ReadString(yamlString);

		Ref<AnimationController> animationController;
		bool result = DeserializeFromYAML(yamlString, animationController);
		if (!result)
			return nullptr;

		return animationController;
	}

	std::string AnimationControllerAssetSerializer::SerializeToYAML(Ref<AnimationController> animationController) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "AnimationController" << YAML::Value;
			out << YAML::BeginMap;
			{
				auto skeleton = animationController->GetSkeletonAsset();
				out << YAML::Key << "SkeletonAsset" << YAML::Value << (skeleton ? skeleton->Handle : 0);
				out << YAML::Key << "States" << YAML::Value;
				out << YAML::BeginSeq;
				{
					for (size_t i = 0; i < animationController->GetNumStates(); ++i)
					{
						out << YAML::BeginMap;
						{
							auto state = animationController->GetAnimationState(i);
							out << YAML::Key << "Name" << YAML::Value << animationController->GetStateName(i);
							auto animation = state->GetAnimationAsset();
							out << YAML::Key << "AnimationAsset" << YAML::Value << (animation ? animation->Handle : 0);
							out << YAML::Key << "AnimationIndex" << YAML::Value << state->GetAnimationIndex();
							out << YAML::Key << "Loop" << YAML::Value << state->IsLooping();
							out << YAML::Key << "RootTranslationMask" << YAML::Value << 1.0f - state->GetRootTranslationMask();
							out << YAML::Key << "RootRotationMask" << YAML::Value << 1.0f - state->GetRootRotationMask();
							out << YAML::Key << "RootTranslationExtractMask" << YAML::Value << state->GetRootTranslationExtractMask();
							out << YAML::Key << "RootRotationExtractMask" << YAML::Value << state->GetRootRotationExtractMask();
						}
						out << YAML::EndMap;
					}
				}
				out << YAML::EndSeq;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		return std::string(out.c_str());
	}

	bool AnimationControllerAssetSerializer::DeserializeFromYAML(const std::string& yamlString, Ref<AnimationController>& animationController) const
	{
		YAML::Node data = YAML::Load(yamlString);
		if (!data["AnimationController"])
			return false;

		YAML::Node animationControllerNode = data["AnimationController"];

		animationController = Ref<AnimationController>::Create();

		// We could be re-loading AnimationController, in which case we want to make sure we also attempt a reload of the child assets (skeleton, animations, etc.)
		// (because at the moment, there is no way to (re)load those child assets individually)
		auto skeletonAssetHandle = animationControllerNode["SkeletonAsset"].as<uint64_t>(0);
		AssetManager::ReloadData(skeletonAssetHandle);
		auto skeletonAsset = AssetManager::GetAsset<SkeletonAsset>(skeletonAssetHandle);
		animationController->SetSkeletonAsset(skeletonAsset);
		for (auto& stateNode : animationControllerNode["States"])
		{
			if (!stateNode.IsMap() || !stateNode["AnimationAsset"] || !stateNode["Name"])
				continue;

			auto state = Ref<AnimationState>::Create();

			// We could be re-loading AnimationController, in which case we want to make sure we also attempt a reload of the child assets (skeleton, animations, etc.)
			// (because at the moment, there is no way to (re)load those child assets individually)
			auto animationAssetHandle = stateNode["AnimationAsset"].as<uint64_t>(0);
			if (AssetManager::IsAssetHandleValid(animationAssetHandle))
				AssetManager::ReloadData(animationAssetHandle);

			auto animationAsset = AssetManager::GetAsset<AnimationAsset>(animationAssetHandle);
			state->SetAnimationAsset(animationAsset);
			state->SetAnimationIndex(stateNode["AnimationIndex"].as<uint32_t>(state->GetAnimationIndex()));
			state->SetIsLooping(stateNode["Loop"].as<bool>(state->IsLooping()));
			state->SetRootTranslationMask(1.0f - stateNode["RootTranslationMask"].as<glm::vec3>(state->GetRootTranslationMask()));
			state->SetRootRotationMask(1.0f - stateNode["RootRotationMask"].as<float>(state->GetRootRotationMask()));
			state->SetRootTranslationExtractMask(stateNode["RootTranslationExtractMask"].as<glm::vec3>(state->GetRootTranslationExtractMask()));
			state->SetRootRotationExtractMask(stateNode["RootRotationExtractMask"].as<float>(state->GetRootRotationExtractMask()));
			state->SetValid(
				animationAsset &&
				animationAsset->GetMeshSource() &&
				animationAsset->GetMeshSource()->IsValid() &&
				skeletonAsset &&
				skeletonAsset->IsValid() &&
				animationAsset->GetMeshSource()->IsCompatibleSkeleton(state->GetAnimationIndex(), skeletonAsset->GetSkeleton())
			);
			if (!state->IsValid()) {
				ANT_CORE_WARN("Invalid animation state: '{0}': {1}",
					stateNode["Name"].as<std::string>(),
					animationAsset
					? animationAsset->GetMeshSource()
					? animationAsset->GetMeshSource()->IsValid()
					? skeletonAsset
					? skeletonAsset->IsValid()
					? "Incompatible Skeleton"
					: "SkeletonAsset is invalid"
					: "SkeletonAsset is null"
					: "MeshSource is invalid"
					: "MeshSource is null"
					: "AnimationAsset is null"
				);
			}

			animationController->AddState(stateNode["Name"].as<std::string>(), state);
		}

		return true;
	}

}