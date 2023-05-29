#pragma once

#include "Ant/Core/Base.h"
#include "PhysXUtils.h"
#include "Ant/Renderer/Mesh.h"
#include "Ant/Asset/MeshColliderAsset.h"

namespace Ant{

	enum class MeshColliderType : uint8_t { Triangle = 0, Convex = 1, None = 3 };

	struct SubmeshColliderData
	{
		Buffer ColliderData;
		glm::mat4 Transform;
	};

	struct MeshColliderData
	{
		std::vector<SubmeshColliderData> Submeshes;
		MeshColliderType Type;
	};

	class CookingFactory
	{
	public:
		static void Initialize();
		static void Shutdown();

		static std::pair<CookingResult, CookingResult> CookMesh(AssetHandle colliderHandle, bool invalidateOld = false);
		static std::pair<CookingResult, CookingResult> CookMesh(Ref<MeshColliderAsset> colliderAsset, bool invalidateOld = false);
	private:
		static bool SerializeMeshCollider(const std::filesystem::path& filepath, MeshColliderData& meshData);
		static MeshColliderData DeserializeMeshCollider(const std::filesystem::path& filepath);
		static CookingResult CookConvexMesh(const Ref<MeshColliderAsset>& colliderAsset, const Ref<MeshSource>& meshSource, const std::vector<uint32_t>& submeshIndices, MeshColliderData& outData);
		static CookingResult CookTriangleMesh(const Ref<MeshColliderAsset>& colliderAsset, const Ref<MeshSource>& meshSource, const std::vector<uint32_t>& submeshIndices, MeshColliderData& outData);
		static void GenerateDebugMesh(const Ref<MeshColliderAsset>& colliderAsset, const MeshColliderData& colliderData);
	};
}
