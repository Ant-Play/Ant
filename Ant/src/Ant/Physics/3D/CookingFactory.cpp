#include "antpch.h"
#include "CookingFactory.h"
#include "PhysicsSystem.h"
#include "PhysXInternal.h"

#include "Ant/Asset/AssetManager.h"
#include "Ant/Project/Project.h"
#include "Ant/Math/Math.h"
#include "Ant/Core/Timer.h"

#include <filesystem>

namespace Ant {

	struct CookingData
	{
		physx::PxCooking* CookingSDK;
		physx::PxCookingParams DefaultCookingParameters;

		CookingData(const physx::PxTolerancesScale& scale)
			: CookingSDK(nullptr), DefaultCookingParameters(scale)
		{
		}
	};

	// .amc file format
	struct AntPhysicsMesh
	{
		const char Header[6] = "AntMC";
		MeshColliderType Type;
		uint32_t SubmeshCount;
	};

	static CookingData* s_CookingData = nullptr;
	void CookingFactory::Initialize()
	{
		s_CookingData = anew CookingData(PhysXInternal::GetPhysXSDK().getTolerancesScale());
		s_CookingData->DefaultCookingParameters.midphaseDesc = physx::PxMeshMidPhase::eBVH34;

		s_CookingData->CookingSDK = PxCreateCooking(PX_PHYSICS_VERSION, PhysXInternal::GetFoundation(), s_CookingData->DefaultCookingParameters);
		ANT_CORE_ASSERT(s_CookingData->CookingSDK, "Couldn't initialize PhysX Cooking SDK!");
	}

	void CookingFactory::Shutdown()
	{
		s_CookingData->CookingSDK->release();
		s_CookingData->CookingSDK = nullptr;

		delete s_CookingData;
	}

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

	std::pair<CookingResult, CookingResult> CookingFactory::CookMesh(AssetHandle colliderHandle, bool invalidateOld)
	{
		return CookMesh(AssetManager::GetAsset<MeshColliderAsset>(colliderHandle), invalidateOld);
	}

	std::pair<CookingResult, CookingResult> CookingFactory::CookMesh(Ref<MeshColliderAsset> colliderAsset, bool invalidateOld /*= false*/)
	{
		ANT_SCOPE_TIMER("CookingFactory::CookMesh");

		Utils::CreateCacheDirectoryIfNeeded();

		AssetHandle colliderHandle = colliderAsset->Handle;
		//const auto& meshMetadata = Project::GetEditorAssetManager()->GetMetadata(colliderAsset->ColliderMesh);

		if (!AssetManager::IsAssetHandleValid(colliderHandle))
		{
			ANT_CORE_ERROR("Invalid mesh");
			return { CookingResult::InvalidMesh, CookingResult::InvalidMesh };
		}

		Ref<Asset> mesh = AssetManager::GetAsset<Asset>(colliderAsset->ColliderMesh);

		if (!mesh)
		{
			ANT_CORE_ERROR_TAG("CookingFactory", "Failed to cook mesh collider because mesh can't be found!");
			return { CookingResult::InvalidMesh, CookingResult::InvalidMesh };
		}

		ANT_CORE_ASSERT(mesh->GetAssetType() == AssetType::StaticMesh || mesh->GetAssetType() == AssetType::Mesh);
		bool isStaticMesh = mesh->GetAssetType() == AssetType::StaticMesh;

		if (!mesh)
		{
			return { CookingResult::InvalidMesh, CookingResult::InvalidMesh };
		}

		// Editor: 
		// name-handle.amc
		// std::string baseFileName = fmt::format("{0}-{1}", meshMetadata.FilePath.stem().string(), meshMetadata.Handle);

		// Runtime:
		std::string baseFileName = fmt::format("{0}-{1}", "Mesh", mesh->Handle);

		const bool isPhysicalAsset = !AssetManager::IsMemoryAsset(colliderHandle);

		// NOTE(Peter): Collider Asset is a real asset on disk, so we need to save it's data since it's based on the settings provided by the asset
		if (isPhysicalAsset)
		{
			// Editor:
			// baseFileName = fmt::format("{0}-{1}", meshMetadata.FilePath.stem().string(), colliderHandle);
			// Runtime:
			baseFileName = fmt::format("{0}-{1}", "Mesh", colliderHandle);
		}

		std::filesystem::path simpleColliderFilePath = Utils::GetCacheDirectory() / fmt::format("{0}-Simple.amc", baseFileName);

		CachedColliderData colliderData;
		CookingResult simpleMeshResult = CookingResult::Failure;
		CookingResult complexMeshResult = CookingResult::Failure;

		Ref<MeshSource> meshSource = isStaticMesh ? mesh.As<StaticMesh>()->GetMeshSource() : mesh.As<Mesh>()->GetMeshSource();
		const auto& submeshIndices = isStaticMesh ? mesh.As<StaticMesh>()->GetSubmeshes() : mesh.As<Mesh>()->GetSubmeshes();

		// Cook or load the simple collider
		{
			// NOTE(Peter): We always try and do this because complex colliders can't be triggers
			if (invalidateOld || !std::filesystem::exists(simpleColliderFilePath))
			{
				simpleMeshResult = CookConvexMesh(colliderAsset, meshSource, submeshIndices, colliderData.SimpleColliderData);

				if (simpleMeshResult == CookingResult::Success && !SerializeMeshCollider(simpleColliderFilePath, colliderData.SimpleColliderData))
				{
					ANT_CORE_ERROR_TAG("Physics", "Failed to cook simple collider mesh, aborting...");
					simpleMeshResult = CookingResult::Failure;
				}
			}
			else
			{
				colliderData.SimpleColliderData = DeserializeMeshCollider(simpleColliderFilePath);
				simpleMeshResult = CookingResult::Success;
			}

#ifndef ANT_DIST
			if (simpleMeshResult == CookingResult::Success)
				GenerateDebugMesh(colliderAsset, colliderData.SimpleColliderData);
#endif
		}

		// Cook or load complex collider mesh
		{
			if (colliderAsset->CollisionComplexity != ECollisionComplexity::UseSimpleAsComplex || simpleMeshResult == CookingResult::Failure)
			{
				std::filesystem::path complexColliderFilePath = Utils::GetCacheDirectory() / fmt::format("{0}-Complex.amc", baseFileName);

				if (invalidateOld || !std::filesystem::exists(complexColliderFilePath))
				{
					complexMeshResult = CookTriangleMesh(colliderAsset, meshSource, submeshIndices, colliderData.ComplexColliderData);

					if (complexMeshResult == CookingResult::Success && !SerializeMeshCollider(complexColliderFilePath, colliderData.ComplexColliderData))
					{
						ANT_CORE_WARN_TAG("Physics", "Failed to cook complex collider mesh, using simple for everything");
						complexMeshResult = CookingResult::Failure;
					}
				}
				else
				{
					colliderData.ComplexColliderData = DeserializeMeshCollider(complexColliderFilePath);
					complexMeshResult = CookingResult::Success;
				}

				if (complexMeshResult == CookingResult::Success)
				{
#ifndef ANT_DIST
					GenerateDebugMesh(colliderAsset, colliderData.ComplexColliderData);
#endif

					if (simpleMeshResult != CookingResult::Success)
					{
						colliderAsset->CollisionComplexity = ECollisionComplexity::UseComplexAsSimple;

						if (isPhysicalAsset)
							AssetImporter::Serialize(colliderAsset);
					}
				}
			}
		}

		if (simpleMeshResult == CookingResult::Success || complexMeshResult == CookingResult::Success)
		{
			// Add to cache
			auto& meshCache = PhysicsSystem::GetMeshCache();
			if (isPhysicalAsset)
				meshCache.m_MeshData[colliderAsset->ColliderMesh][colliderHandle] = colliderData;
			else
				meshCache.m_MeshData[colliderAsset->ColliderMesh][0] = colliderData;
		}

		return { simpleMeshResult, complexMeshResult };
	}

	bool CookingFactory::SerializeMeshCollider(const std::filesystem::path& filepath, MeshColliderData& meshData)
	{
		AntPhysicsMesh amc;
		amc.Type = meshData.Type;
		amc.SubmeshCount = (uint32_t)meshData.Submeshes.size();

		std::ofstream stream(filepath, std::ios::binary | std::ios::trunc);
		if (!stream)
		{
			stream.close();
			ANT_CORE_ERROR("Failed to write collider to {0}", filepath.string());
			for (auto& submesh : meshData.Submeshes)
				submesh.ColliderData.Release();
			meshData.Submeshes.clear();
			return false;
		}

		stream.write((char*)&amc, sizeof(AntPhysicsMesh));
		for (auto& submesh : meshData.Submeshes)
		{
			stream.write((char*)glm::value_ptr(submesh.Transform), sizeof(submesh.Transform));
			stream.write((char*)&submesh.ColliderData.Size, sizeof(submesh.ColliderData.Size));
			stream.write((char*)submesh.ColliderData.Data, submesh.ColliderData.Size);
		}

		return true;
	}

	MeshColliderData CookingFactory::DeserializeMeshCollider(const std::filesystem::path& filepath)
	{
		// Deserialize
		Buffer colliderBuffer = FileSystem::ReadBytes(filepath);
		AntPhysicsMesh& amc = *(AntPhysicsMesh*)colliderBuffer.Data;
		ANT_CORE_VERIFY(strcmp(amc.Header, AntPhysicsMesh().Header) == 0);

		auto& meshCache = PhysicsSystem::GetMeshCache();
		MeshColliderData meshData;
		meshData.Type = amc.Type;
		meshData.Submeshes.resize(amc.SubmeshCount);

		uint8_t* buffer = colliderBuffer.As<uint8_t>() + sizeof(AntPhysicsMesh);
		for (uint32_t i = 0; i < amc.SubmeshCount; i++)
		{
			SubmeshColliderData& submeshData = meshData.Submeshes[i];

			// Transform
			memcpy(glm::value_ptr(submeshData.Transform), buffer, sizeof(glm::mat4));
			buffer += sizeof(glm::mat4);

			// Data
			uint64_t size = *(uint64_t*)buffer;
			buffer += sizeof(uint64_t);
			submeshData.ColliderData.Release();
			submeshData.ColliderData = Buffer::Copy(buffer, size);
			buffer += size;
		}

		colliderBuffer.Release();
		return meshData;
	}

	CookingResult CookingFactory::CookConvexMesh(const Ref<MeshColliderAsset>& colliderAsset, const Ref<MeshSource>& meshSource, const std::vector<uint32_t>& submeshIndices, MeshColliderData& outData)
	{
		physx::PxConvexMeshDesc convexDesc;
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		// Update cooking parameters
		physx::PxCookingParams cookingParams(s_CookingData->DefaultCookingParameters);
		if (colliderAsset->EnableVertexWelding)
		{
			cookingParams.meshPreprocessParams = physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
			cookingParams.meshWeldTolerance = colliderAsset->VertexWeldTolerance;
		}

		if (colliderAsset->CheckZeroAreaTriangles)
		{
			convexDesc.flags |= physx::PxConvexFlag::eCHECK_ZERO_AREA_TRIANGLES;
			float zeroAreaTriangleThreshold = colliderAsset->AreaTestEpsilon;

			if (zeroAreaTriangleThreshold <= 0.0f)
			{
				ANT_CONSOLE_LOG_WARN("Attempting to cook convex mesh collider with a zero-area threshold of {}! Zero-Area Threshold has to be greater than 0! Using 0.01 instead.", zeroAreaTriangleThreshold);
				zeroAreaTriangleThreshold = 0.01f;
			}

			cookingParams.areaTestEpsilon = zeroAreaTriangleThreshold;
		}

		if (colliderAsset->ShiftVerticesToOrigin)
			convexDesc.flags |= physx::PxConvexFlag::eSHIFT_VERTICES;

		s_CookingData->CookingSDK->setParams(cookingParams);

		const auto& vertices = meshSource->GetVertices();
		const auto& indices = meshSource->GetIndices();
		const auto& submeshes = meshSource->GetSubmeshes();

		CookingResult cookingResult = CookingResult::Failure;

		for (auto submeshIndex : submeshIndices)
		{
			const auto& submesh = submeshes[submeshIndex];

			convexDesc.points.count = submesh.VertexCount;
			convexDesc.points.stride = sizeof(Vertex);
			convexDesc.points.data = &vertices[submesh.BaseVertex];
			convexDesc.indices.count = submesh.IndexCount / 3;
			convexDesc.indices.data = &indices[submesh.BaseIndex / 3];
			convexDesc.indices.stride = sizeof(Index);

			if (vertices.size() >= convexDesc.vertexLimit)
			{
				ANT_CORE_INFO_TAG("Physics", "Attempting to cook a convex submesh that has more than {0} vertices! Switching to quantizing the input vertices.", convexDesc.vertexLimit);
				convexDesc.flags |= physx::PxConvexFlag::eQUANTIZE_INPUT | physx::PxConvexFlag::eSHIFT_VERTICES;
				convexDesc.quantizedCount = (physx::PxU16)vertices.size(); // TODO: This should be the vertex count for this submesh, not the entire mesh hierarchy
			}
			else
			{
				// Disable input quantization
				convexDesc.flags &= ~physx::PxConvexFlag::eQUANTIZE_INPUT;
			}

			physx::PxDefaultMemoryOutputStream buf;
			physx::PxConvexMeshCookingResult::Enum oldResult = physx::PxConvexMeshCookingResult::eSUCCESS;
			physx::PxConvexMeshCookingResult::Enum result;

			if (!s_CookingData->CookingSDK->cookConvexMesh(convexDesc, buf, &result))
			{

				bool fatalError = true;

				if (result == physx::PxConvexMeshCookingResult::eZERO_AREA_TEST_FAILED)
				{
					if (!Application::IsRuntime())
					{
						const auto& meshMetadata = Project::GetEditorAssetManager()->GetMetadata(colliderAsset->ColliderMesh);
						ANT_CONSOLE_LOG_WARN("Failed to cook submesh '{}' in mesh '{}'! Triangle with an area of 0 detected! Attempting to cook submesh without checking for zero-area triangles. If you experience issues with this mesh collider you should make sure your mesh doesn't have small triangles!", submesh.MeshName, meshMetadata.FilePath.string());
					}

					float previousZeroAreaThreshold = cookingParams.areaTestEpsilon;
					convexDesc.flags &= ~physx::PxConvexFlag::eCHECK_ZERO_AREA_TRIANGLES;
					// Don't set to 0 here, it's invalid
					cookingParams.areaTestEpsilon = 0.00001f;
					s_CookingData->CookingSDK->setParams(cookingParams);

					// Attempt to cook without checking zero-area triangles
					oldResult = result;
					fatalError = !s_CookingData->CookingSDK->cookConvexMesh(convexDesc, buf, &result);

					cookingParams.areaTestEpsilon = previousZeroAreaThreshold;
					convexDesc.flags |= physx::PxConvexFlag::eCHECK_ZERO_AREA_TRIANGLES;
					s_CookingData->CookingSDK->setParams(cookingParams);
				}

				if (fatalError)
				{
					std::string errorMessage = "Unknown Error!";

					switch (result)
					{
					case physx::PxConvexMeshCookingResult::ePOLYGONS_LIMIT_REACHED:
					{
						errorMessage = fmt::format("Submesh has more than {} vertices! This shouldn't have been a problem because Ant should've detected this prior to cooking this submesh!", convexDesc.quantizedCount);
						break;
					}
					}

					if (!Application::IsRuntime())
					{
						const auto& meshMetadata = Project::GetEditorAssetManager()->GetMetadata(colliderAsset->ColliderMesh);
						ANT_CONSOLE_LOG_ERROR("Failed to cook submesh '{}' in mesh '{}'! Error: {}", submesh.MeshName, meshMetadata.FilePath.string(), errorMessage);
					}

					for (auto& existingSubmesh : outData.Submeshes)
						existingSubmesh.ColliderData.Release();
					outData.Submeshes.clear();
					cookingResult = PhysXUtils::FromPhysXCookingResult(result);
					break;
				}
			}

			SubmeshColliderData& data = outData.Submeshes.emplace_back();
			data.ColliderData = Buffer::Copy(buf.getData(), buf.getSize());
			data.Transform = submesh.Transform * glm::scale(glm::mat4(1.0f), colliderAsset->ColliderScale);
			cookingResult = CookingResult::Success;
		}

		s_CookingData->CookingSDK->setParams(s_CookingData->DefaultCookingParameters);
		outData.Type = MeshColliderType::Convex;
		return cookingResult;
	}

	CookingResult CookingFactory::CookTriangleMesh(const Ref<MeshColliderAsset>& colliderAsset, const Ref<MeshSource>& meshSource, const std::vector<uint32_t>& submeshIndices, MeshColliderData& outData)
	{
		// Update cooking parameters
		physx::PxCookingParams cookingParams(s_CookingData->DefaultCookingParameters);

		if (colliderAsset->EnableVertexWelding)
		{
			cookingParams.meshPreprocessParams = physx::PxMeshPreprocessingFlag::eWELD_VERTICES;
			cookingParams.meshWeldTolerance = colliderAsset->VertexWeldTolerance;
		}
		s_CookingData->CookingSDK->setParams(cookingParams);

		physx::PxTriangleMeshDesc triangleDesc;

		if (colliderAsset->FlipNormals)
			triangleDesc.flags = physx::PxMeshFlag::eFLIPNORMALS;

		CookingResult cookingResult = CookingResult::Failure;

		const auto& vertices = meshSource->GetVertices();
		const auto& indices = meshSource->GetIndices();
		const auto& submeshes = meshSource->GetSubmeshes();

		for (auto submeshIndex : submeshIndices)
		{
			const auto& submesh = submeshes[submeshIndex];

			triangleDesc.points.stride = sizeof(Vertex);
			triangleDesc.points.count = submesh.VertexCount;
			triangleDesc.points.data = &vertices[submesh.BaseVertex];
			triangleDesc.triangles.stride = sizeof(Index);
			triangleDesc.triangles.count = submesh.IndexCount / 3;
			triangleDesc.triangles.data = &indices[submesh.BaseIndex / 3];

			physx::PxDefaultMemoryOutputStream buf;
			physx::PxTriangleMeshCookingResult::Enum result;
			if (!s_CookingData->CookingSDK->cookTriangleMesh(triangleDesc, buf, &result))
			{
				ANT_CORE_ERROR("Failed to cook static mesh {0}", meshSource->GetFilePath());

				for (auto& existingSubmesh : outData.Submeshes)
					existingSubmesh.ColliderData.Release();
				outData.Submeshes.clear();
				cookingResult = PhysXUtils::FromPhysXCookingResult(result);
				break;
			}

			SubmeshColliderData& data = outData.Submeshes.emplace_back();
			data.ColliderData = Buffer::Copy(buf.getData(), buf.getSize());
			data.Transform = submesh.Transform * glm::scale(glm::mat4(1.0f), colliderAsset->ColliderScale);
			cookingResult = CookingResult::Success;
		}

		s_CookingData->CookingSDK->setParams(s_CookingData->DefaultCookingParameters);
		outData.Type = MeshColliderType::Triangle;
		return cookingResult;
	}

#ifndef ANT_DIST
	void CookingFactory::GenerateDebugMesh(const Ref<MeshColliderAsset>& colliderAsset, const MeshColliderData& colliderData)
	{
		if (colliderData.Type == MeshColliderType::Triangle && colliderAsset->CollisionComplexity != ECollisionComplexity::UseSimpleAsComplex)
		{
			std::vector<Vertex> vertices;
			std::vector<Index> indices;
			std::vector<Submesh> submeshes;
			for (size_t i = 0; i < colliderData.Submeshes.size(); i++)
			{
				const SubmeshColliderData& submeshData = colliderData.Submeshes[i];

				physx::PxDefaultMemoryInputData input(submeshData.ColliderData.As<physx::PxU8>(), submeshData.ColliderData.Size);
				physx::PxTriangleMesh* trimesh = PhysXInternal::GetPhysXSDK().createTriangleMesh(input);

				if (!trimesh)
					continue;

				const uint32_t nbVerts = trimesh->getNbVertices();
				const physx::PxVec3* triangleVertices = trimesh->getVertices();
				const uint32_t nbTriangles = trimesh->getNbTriangles();
				const physx::PxU16* tris = (const physx::PxU16*)trimesh->getTriangles();

				vertices.reserve(vertices.size() + nbVerts);
				indices.reserve(indices.size() + nbTriangles);

				Submesh& submesh = submeshes.emplace_back();
				submesh.BaseVertex = static_cast<uint32_t>(vertices.size());
				submesh.VertexCount = nbVerts;
				submesh.BaseIndex = static_cast<uint32_t>(indices.size()) * 3;
				submesh.IndexCount = nbTriangles * 3;
				submesh.MaterialIndex = 0;
				submesh.Transform = submeshData.Transform;

				for (uint32_t v = 0; v < nbVerts; v++)
				{
					Vertex& v1 = vertices.emplace_back();
					v1.Position = PhysXUtils::FromPhysXVector(triangleVertices[v]);
				}

				for (uint32_t tri = 0; tri < nbTriangles; tri++)
				{
					Index& index = indices.emplace_back();
					index.V1 = tris[3 * tri + 0];
					index.V2 = tris[3 * tri + 1];
					index.V3 = tris[3 * tri + 2];
				}

				trimesh->release();
			}

			if (vertices.size() > 0)
			{
				Ref<MeshSource> meshAsset = Ref<MeshSource>::Create(vertices, indices, submeshes);
				PhysicsSystem::GetMeshCache().AddDebugMesh(colliderAsset, Ref<StaticMesh>::Create(meshAsset));
			}
		}
		else
		{
			std::vector<Vertex> vertices;
			std::vector<Index> indices;
			std::vector<Submesh> submeshes;

			for (size_t i = 0; i < colliderData.Submeshes.size(); i++)
			{
				const SubmeshColliderData& submeshData = colliderData.Submeshes[i];

				physx::PxDefaultMemoryInputData input(submeshData.ColliderData.As<physx::PxU8>(), submeshData.ColliderData.Size);
				physx::PxConvexMesh* convexMesh = PhysXInternal::GetPhysXSDK().createConvexMesh(input);

				if (!convexMesh)
					continue;

				// Based On: https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/ThirdParty/PhysX3/NvCloth/samples/SampleBase/renderer/ConvexRenderMesh.cpp
				const uint32_t nbPolygons = convexMesh->getNbPolygons();
				const physx::PxVec3* convexVertices = convexMesh->getVertices();
				const physx::PxU8* convexIndices = convexMesh->getIndexBuffer();

				uint32_t nbVertices = 0;
				uint32_t nbFaces = 0;
				uint32_t vertCounter = 0;
				uint32_t indexCounter = 0;

				Submesh& submesh = submeshes.emplace_back();
				submesh.BaseVertex = static_cast<uint32_t>(vertices.size());
				submesh.BaseIndex = static_cast<uint32_t>(indices.size()) * 3;

				for (uint32_t i = 0; i < nbPolygons; i++)
				{
					physx::PxHullPolygon polygon;
					convexMesh->getPolygonData(i, polygon);
					nbVertices += polygon.mNbVerts;
					nbFaces += (polygon.mNbVerts - 2) * 3;

					uint32_t vI0 = vertCounter;
					for (uint32_t vI = 0; vI < polygon.mNbVerts; vI++)
					{
						Vertex& v = vertices.emplace_back();
						v.Position = PhysXUtils::FromPhysXVector(convexVertices[convexIndices[polygon.mIndexBase + vI]]);
						vertCounter++;
					}

					for (uint32_t vI = 1; vI < uint32_t(polygon.mNbVerts) - 1; vI++)
					{
						Index& index = indices.emplace_back();
						index.V1 = uint32_t(vI0);
						index.V2 = uint32_t(vI0 + vI + 1);
						index.V3 = uint32_t(vI0 + vI);
						indexCounter++;
					}
				}

				submesh.VertexCount = vertCounter;
				submesh.IndexCount = indexCounter * 3;
				submesh.MaterialIndex = 0;
				submesh.Transform = submeshData.Transform;

				convexMesh->release();
			}

			if (vertices.size() > 0)
			{
				Ref<MeshSource> meshAsset = Ref<MeshSource>::Create(vertices, indices, submeshes);
				PhysicsSystem::GetMeshCache().AddDebugMesh(colliderAsset, Ref<Mesh>::Create(meshAsset));
			}
		}

	}
#else
	void CookingFactory::GenerateDebugMesh(const Ref<MeshColliderAsset>& colliderAsset, const MeshColliderData& colliderData) {}
#endif
}
