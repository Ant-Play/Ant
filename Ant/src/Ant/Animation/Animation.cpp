#include "antpch.h"
#include "Animation.h"
#include "AnimationImporterAssimp.h"

#include "Ant/Asset/AssetManager.h"
#include "Ant/Renderer/Mesh.h"

namespace Ant{

	Animation::Animation(const std::string_view name, const float duration)
		: m_Name(name)
		, m_Duration(duration)
	{}

	void Animation::SetKeyFrames(std::vector<TranslationKey> translations, std::vector<RotationKey> rotations, std::vector<ScaleKey> scales)
	{
		m_TranslationKeys = std::move(translations);
		m_RotationKeys = std::move(rotations);
		m_ScaleKeys = std::move(scales);
	}

	AnimationAsset::AnimationAsset(const UUID meshSource) : m_MeshSource(AssetManager::GetAsset<MeshSource>(meshSource))
	{
	}

	AnimationAsset::AnimationAsset(const Ref<MeshSource> meshSource) : m_MeshSource(meshSource)
	{
	}

	Ref<MeshSource> AnimationAsset::GetMeshSource() const
	{
		return m_MeshSource;
	}

	uint32_t AnimationAsset::GetAnimationCount() const
	{
		return m_MeshSource->GetAnimationCount();
	}

	const Animation& AnimationAsset::GetAnimation(const uint32_t animationIndex, const Skeleton& skeleton) const
	{
		return m_MeshSource->GetAnimation(animationIndex, skeleton);
	}

}