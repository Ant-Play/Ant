#include "antpch.h"
#include "Skeleton.h"

#include "Ant/Asset/AssetManager.h"
#include "Ant/Math/Math.h"
#include "Ant/Renderer/Mesh.h"

namespace Ant{

	Skeleton::Skeleton(uint32_t size)
	{
		m_BoneNames.reserve(size);
		m_ParentBoneIndices.reserve(size);
	}

	uint32_t Skeleton::AddBone(std::string name, uint32_t parentIndex, const glm::mat4& transform)
	{
		uint32_t index = static_cast<uint32_t>(m_BoneNames.size());
		m_BoneNames.emplace_back(name);
		m_ParentBoneIndices.emplace_back(parentIndex);
		m_BoneTranslations.emplace_back();
		m_BoneRotations.emplace_back();
		m_BoneScales.emplace_back();
		Math::DecomposeTransform(transform, m_BoneTranslations.back(), m_BoneRotations.back(), m_BoneScales.back());

		return index;
	}

	uint32_t Skeleton::GetBoneIndex(const std::string_view name) const
	{
		for (size_t i = 0; i < m_BoneNames.size(); ++i)
		{
			if (m_BoneNames[i] == name)
			{
				return static_cast<uint32_t>(i);
			}
		}
		return Skeleton::NullIndex;
	}

	bool Skeleton::operator ==(const Skeleton& other) const
	{
		bool areSame = false;
		if (GetNumBones() == other.GetNumBones())
		{
			areSame = true;
			for (uint32_t i = 0; i < GetNumBones(); ++i)
			{
				if (GetBoneName(i) != other.GetBoneName(i))
				{
					areSame = false;
					break;
				}
			}
		}
		return areSame;
	}

	SkeletonAsset::SkeletonAsset(const UUID meshSource) : m_MeshSource(AssetManager::GetAsset<MeshSource>(meshSource))
	{
	}

	SkeletonAsset::SkeletonAsset(const Ref<MeshSource> meshSource) : m_MeshSource(meshSource)
	{
	}

	Ref<MeshSource> SkeletonAsset::GetMeshSource() const
	{
		return m_MeshSource;
	}

	const Skeleton& SkeletonAsset::GetSkeleton() const
	{
		ANT_CORE_ASSERT(m_MeshSource && m_MeshSource->HasSkeleton());
		return m_MeshSource->GetSkeleton();
	}

}