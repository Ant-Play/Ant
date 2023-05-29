#include "antpch.h"
#include "Mesh.h"

#include "Ant/Debug/Profiler.h"
#include "Ant/Math/Math.h"
#include "Ant/Renderer/Renderer.h"
#include "Ant/Project/Project.h"
#include "Ant/Asset/AssimpMeshImporter.h"
#include "Ant/Asset/AssetManager.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "imgui/imgui.h"

#include <filesystem>

namespace Ant{

#define MESH_DEBUG_LOG 0
#if MESH_DEBUG_LOG
#define ANT_MESH_LOG(...) ANT_CORE_TRACE_TAG("Mesh", __VA_ARGS__)
#define ANT_MESH_ERROR(...) ANT_CORE_ERROR_TAG("Mesh", __VA_ARGS__)
#else
#define ANT_MESH_LOG(...)
#define ANT_MESH_ERROR(...)
#endif

	////////////////////////////////////////////////////////
	// MeshSource //////////////////////////////////////////
	////////////////////////////////////////////////////////

	MeshSource::MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform)
		: m_Vertices(vertices), m_Indices(indices)
	{
		// Generate a new asset handle
		Handle = {};

		Submesh submesh;
		submesh.BaseVertex = 0;
		submesh.BaseIndex = 0;
		submesh.IndexCount = (uint32_t)indices.size() * 3u;
		submesh.Transform = transform;
		m_Submeshes.push_back(submesh);

		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), (uint32_t)(m_Vertices.size() * sizeof(Vertex)));
		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), (uint32_t)(m_Indices.size() * sizeof(Index)));
	}

	MeshSource::MeshSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const std::vector<Submesh>& submeshes)
		: m_Vertices(vertices), m_Indices(indices), m_Submeshes(submeshes)
	{
		// Generate a new asset handle
		Handle = {};

		m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(), (uint32_t)(m_Vertices.size() * sizeof(Vertex)));
		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), (uint32_t)(m_Indices.size() * sizeof(Index)));

		// TODO: generate bounding box for submeshes, etc.
	}

	MeshSource::~MeshSource()
	{
	}

	static std::string LevelToSpaces(uint32_t level)
	{
		std::string result = "";
		for (uint32_t i = 0; i < level; i++)
			result += "--";
		return result;
	}

	void MeshSource::DumpVertexBuffer()
	{
		// TODO: Convert to ImGui
		ANT_MESH_LOG("------------------------------------------------------");
		ANT_MESH_LOG("Vertex Buffer Dump");
		ANT_MESH_LOG("Mesh: {0}", m_FilePath);
		for (size_t i = 0; i < m_Vertices.size(); i++)
		{
			auto& vertex = m_Vertices[i];
			ANT_MESH_LOG("Vertex: {0}", i);
			ANT_MESH_LOG("Position: {0}, {1}, {2}", vertex.Position.x, vertex.Position.y, vertex.Position.z);
			ANT_MESH_LOG("Normal: {0}, {1}, {2}", vertex.Normal.x, vertex.Normal.y, vertex.Normal.z);
			ANT_MESH_LOG("Binormal: {0}, {1}, {2}", vertex.Binormal.x, vertex.Binormal.y, vertex.Binormal.z);
			ANT_MESH_LOG("Tangent: {0}, {1}, {2}", vertex.Tangent.x, vertex.Tangent.y, vertex.Tangent.z);
			ANT_MESH_LOG("TexCoord: {0}, {1}", vertex.Texcoord.x, vertex.Texcoord.y);
			ANT_MESH_LOG("--");
		}
		ANT_MESH_LOG("------------------------------------------------------");
	}


	// TODO (0x): this is temporary.. and will eventually be replaced with some kind of skeleton retargeting
	bool MeshSource::IsCompatibleSkeleton(const uint32_t animationIndex, const Skeleton& skeleton) const
	{
		if (!m_Skeleton)
		{
			ANT_CORE_VERIFY(!m_Runtime);
			auto path = Project::GetEditorAssetManager()->GetFileSystemPath(Handle);
			AssimpMeshImporter importer(path);
			return importer.IsCompatibleSkeleton(animationIndex, skeleton);
		}

		return m_Skeleton->GetBoneNames() == skeleton.GetBoneNames();
	}

	uint32_t MeshSource::GetAnimationCount() const
	{
		if (m_Runtime)
			return (uint32_t)m_Animations.size();

		auto path = Project::GetEditorAssetManager()->GetFileSystemPath(Handle);
		AssimpMeshImporter importer(path);
		return importer.GetAnimationCount();
	}

	const Animation& MeshSource::GetAnimation(const uint32_t animationIndex, const Skeleton& skeleton) const
	{
		if (!m_Skeleton)
		{
			ANT_CORE_VERIFY(!m_Runtime);
			auto path = Project::GetEditorAssetManager()->GetFileSystemPath(Handle);
			AssimpMeshImporter importer(path);
			m_Skeleton = CreateScope<Skeleton>(skeleton);
			importer.ImportAnimations(animationIndex, *m_Skeleton, m_Animations);
		}

		ANT_CORE_ASSERT(animationIndex < m_Animations.size(), "Animation index out of range!");
		ANT_CORE_ASSERT(m_Animations[animationIndex], "Attempted to access null animation!");
		return *m_Animations[animationIndex];
	}


	Mesh::Mesh(Ref<MeshSource> meshSource)
		: m_MeshSource(meshSource)
	{
		// Generate a new asset handle
		Handle = {};

		SetSubmeshes({});

		const auto& meshMaterials = meshSource->GetMaterials();
		m_Materials = Ref<MaterialTable>::Create((uint32_t)meshMaterials.size());
		for (size_t i = 0; i < meshMaterials.size(); i++)
			m_Materials->SetMaterial((uint32_t)i, AssetManager::CreateMemoryOnlyAsset<MaterialAsset>(meshMaterials[i]));
	}

	Mesh::Mesh(Ref<MeshSource> meshSource, const std::vector<uint32_t>& submeshes)
		: m_MeshSource(meshSource)
	{
		// Generate a new asset handle
		Handle = {};

		SetSubmeshes(submeshes);

		const auto& meshMaterials = meshSource->GetMaterials();
		m_Materials = Ref<MaterialTable>::Create((uint32_t)meshMaterials.size());
		for (size_t i = 0; i < meshMaterials.size(); i++)
			m_Materials->SetMaterial((uint32_t)i, AssetManager::CreateMemoryOnlyAsset<MaterialAsset>(meshMaterials[i]));
	}

	Mesh::Mesh(const Ref<Mesh>& other)
		: m_MeshSource(other->m_MeshSource), m_Materials(other->m_Materials)
	{
		SetSubmeshes(other->m_Submeshes);
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::SetSubmeshes(const std::vector<uint32_t>& submeshes)
	{
		if (!submeshes.empty())
		{
			m_Submeshes = submeshes;
		}
		else
		{
			const auto& submeshes = m_MeshSource->GetSubmeshes();
			m_Submeshes.resize(submeshes.size());
			for (uint32_t i = 0; i < submeshes.size(); i++)
				m_Submeshes[i] = i;
		}
	}

	////////////////////////////////////////////////////////
	// StaticMesh //////////////////////////////////////////
	////////////////////////////////////////////////////////

	StaticMesh::StaticMesh(Ref<MeshSource> meshSource)
		: m_MeshSource(meshSource)
	{
		// Generate a new asset handle
		Handle = {};

		SetSubmeshes({});

		const auto& meshMaterials = meshSource->GetMaterials();
		uint32_t numMaterials = static_cast<uint32_t>(meshMaterials.size());
		m_Materials = Ref<MaterialTable>::Create(numMaterials);
		for (uint32_t i = 0; i < numMaterials; i++)
			m_Materials->SetMaterial(i, AssetManager::CreateMemoryOnlyAsset<MaterialAsset>(meshMaterials[i]));
	}

	StaticMesh::StaticMesh(Ref<MeshSource> meshSource, const std::vector<uint32_t>& submeshes)
		: m_MeshSource(meshSource)
	{
		// Generate a new asset handle
		Handle = {};

		SetSubmeshes(submeshes);

		const auto& meshMaterials = meshSource->GetMaterials();
		uint32_t numMaterials = static_cast<uint32_t>(meshMaterials.size());
		m_Materials = Ref<MaterialTable>::Create(numMaterials);
		for (uint32_t i = 0; i < numMaterials; i++)
			m_Materials->SetMaterial(i, AssetManager::CreateMemoryOnlyAsset<MaterialAsset>(meshMaterials[i]));
	}

	StaticMesh::StaticMesh(const Ref<StaticMesh>& other)
		: m_MeshSource(other->m_MeshSource), m_Materials(other->m_Materials)
	{
		SetSubmeshes(other->m_Submeshes);
	}

	StaticMesh::~StaticMesh()
	{
	}

	void StaticMesh::SetSubmeshes(const std::vector<uint32_t>& submeshes)
	{
		if (!submeshes.empty())
		{
			m_Submeshes = submeshes;
		}
		else
		{
			const auto& submeshes = m_MeshSource->GetSubmeshes();
			m_Submeshes.resize(submeshes.size());
			for (uint32_t i = 0; i < submeshes.size(); i++)
				m_Submeshes[i] = i;
		}
	}
	
}
