﻿#include "antpch.h"
#include "PhysicsMeshCache.h"

#include "Ant/Asset/AssetManager.h"
#include "Ant/Renderer/MeshFactory.h"

namespace Ant {

	namespace Utils {

		static std::filesystem::path GetCacheDirectory()
		{
			return Project::GetCacheDirectory() / "Colliders";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::filesystem::path cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
				std::filesystem::create_directories(cacheDirectory);
		}
	}

	void PhysicsMeshCache::Init()
	{
		m_BoxMesh = MeshFactory::CreateBox(glm::vec3(1));
		m_SphereMesh = MeshFactory::CreateSphere(0.5f);
		m_CapsuleMesh = MeshFactory::CreateCapsule(0.5f, 1.0f);
	}

	const CachedColliderData& PhysicsMeshCache::GetMeshData(const Ref<MeshColliderAsset>& colliderAsset)
	{
		AssetHandle collisionMesh = colliderAsset->ColliderMesh;

		if (m_MeshData.find(collisionMesh) != m_MeshData.end())
		{
			const auto& meshDataMap = m_MeshData.at(collisionMesh);
			if (meshDataMap.find(colliderAsset->Handle) != meshDataMap.end())
				return meshDataMap.at(colliderAsset->Handle);

			return meshDataMap.at(0);
		}

		// Create/load collision mesh
		auto [simpleMeshResult, complexMeshResult] = CookingFactory::CookMesh(colliderAsset->Handle);
		ANT_CORE_ASSERT(m_MeshData.find(collisionMesh) != m_MeshData.end());
		const auto& meshDataMap = m_MeshData.at(collisionMesh);

		ANT_CORE_ASSERT(meshDataMap.find(colliderAsset->Handle) != meshDataMap.end());
		return meshDataMap.at(colliderAsset->Handle);
	}

	// Debug meshes get created in CookingFactory::CookMesh
	Ref<Mesh> PhysicsMeshCache::GetDebugMesh(const Ref<MeshColliderAsset>& colliderAsset)
	{
		AssetHandle collisionMesh = colliderAsset->ColliderMesh;
		if (m_DebugMeshes.find(collisionMesh) != m_DebugMeshes.end())
		{
			const auto& debugMeshes = m_DebugMeshes.at(collisionMesh);

			if (debugMeshes.find(colliderAsset->Handle) != debugMeshes.end())
				return debugMeshes.at(colliderAsset->Handle);

			if (debugMeshes.find(0) == debugMeshes.end())
				return nullptr;

			return debugMeshes.at(0);
		}

		return nullptr;
	}

	Ref<StaticMesh> PhysicsMeshCache::GetDebugStaticMesh(const Ref<MeshColliderAsset>& colliderAsset)
	{
		AssetHandle collisionMesh = colliderAsset->ColliderMesh;

		if (m_DebugStaticMeshes.find(collisionMesh) != m_DebugStaticMeshes.end())
		{
			const auto& debugMeshes = m_DebugStaticMeshes.at(collisionMesh);

			if (debugMeshes.find(colliderAsset->Handle) != debugMeshes.end())
				return debugMeshes.at(colliderAsset->Handle);

			if (debugMeshes.find(0) == debugMeshes.end())
				return nullptr;

			return debugMeshes.at(0);
		}

		return nullptr;
	}

	bool PhysicsMeshCache::Exists(const Ref<MeshColliderAsset>& colliderAsset) const
	{
		AssetHandle collisionMesh = colliderAsset->ColliderMesh;

		if (m_MeshData.find(collisionMesh) == m_MeshData.end())
			return false;

		const auto& meshDataMap = m_MeshData.at(collisionMesh);
		if (AssetManager::IsMemoryAsset(colliderAsset->Handle))
			return meshDataMap.find(0) != meshDataMap.end();

		return meshDataMap.find(colliderAsset->Handle) != meshDataMap.end();
	}

	void PhysicsMeshCache::Rebuild()
	{
		ANT_CORE_INFO_TAG("Physics", "Rebuilding collider cache");

		if (!FileSystem::DeleteFile(Utils::GetCacheDirectory()))
		{
			ANT_CORE_ERROR_TAG("Physics", "Failed to delete collider cache!");
			return;
		}

		Clear();

		const auto& assetRegistry = Project::GetEditorAssetManager()->GetAssetRegistry();
		const auto& memoryAssets = AssetManager::GetMemoryOnlyAssets();

		for (const auto& [filepath, metadata] : assetRegistry)
		{
			if (metadata.Type != AssetType::MeshCollider)
				continue;

			auto [simpleMeshResult, complexMeshResult] = CookingFactory::CookMesh(metadata.Handle, true);

			if (simpleMeshResult != CookingResult::Success)
				ANT_CORE_ERROR_TAG("Physics", "Failed to cook simple collider for '{0}'", metadata.Handle);

			if (complexMeshResult != CookingResult::Success)
				ANT_CORE_ERROR_TAG("Physics", "Failed to cook complex collider for '{0}'", metadata.Handle);
		}

		for (const auto& [handle, asset] : memoryAssets)
		{
			if (asset->GetAssetType() != AssetType::MeshCollider)
				continue;

			auto [simpleMeshResult, complexMeshResult] = CookingFactory::CookMesh(handle, true);

			if (simpleMeshResult != CookingResult::Success)
				ANT_CORE_ERROR_TAG("Physics", "Failed to cook simple collider for '{0}'", handle);

			if (complexMeshResult != CookingResult::Success)
				ANT_CORE_ERROR_TAG("Physics", "Failed to cook complex collider for '{0}'", handle);
		}

		ANT_CORE_INFO_TAG("Physics", "Finished rebuilding collider cache");
	}

	void PhysicsMeshCache::Clear()
	{
		m_MeshData.clear();
		m_DebugStaticMeshes.clear();
		m_DebugMeshes.clear();
	}

	void PhysicsMeshCache::AddDebugMesh(const Ref<MeshColliderAsset>& colliderAsset, const Ref<StaticMesh>& debugMesh)
	{
		if (AssetManager::IsMemoryAsset(colliderAsset->Handle))
			m_DebugStaticMeshes[colliderAsset->ColliderMesh][0] = debugMesh;
		else
			m_DebugStaticMeshes[colliderAsset->ColliderMesh][colliderAsset->Handle] = debugMesh;
	}

	void PhysicsMeshCache::AddDebugMesh(const Ref<MeshColliderAsset>& colliderAsset, const Ref<Mesh>& debugMesh)
	{
		if (AssetManager::IsMemoryAsset(colliderAsset->Handle))
			m_DebugMeshes[colliderAsset->ColliderMesh][0] = debugMesh;
		else
			m_DebugMeshes[colliderAsset->ColliderMesh][colliderAsset->Handle] = debugMesh;
	}

}