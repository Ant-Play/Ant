#include "antpch.h"
#include "PhysicsShapes.h"
#include "PhysXInternal.h"
#include "PhysicsActor.h"
#include "PhysicsSystem.h"

#include "Ant/Asset/AssetManager.h"
#include "Ant/Math/Math.h"
#include "Ant/Utilities/FileSystem.h"

#include "Ant/Project/Project.h"
#include "Ant/Debug/Profiler.h"

#include <filesystem>

#include <glm/gtc/type_ptr.hpp>

namespace Ant {

	void ColliderShape::SetMaterial(Ref<PhysicsMaterial> material)
	{
		if (m_Material != nullptr)
			m_Material->release();

		m_Material = PhysXInternal::GetPhysXSDK().createMaterial(material->StaticFriction, material->DynamicFriction, material->Bounciness);
	}

	BoxColliderShape::BoxColliderShape(BoxColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::Box, entity)
	{
		Ref<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);

		if (!material)
		{
			component.Material = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>(0.6f, 0.6f, 0.0f);
			material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);
		}

		SetMaterial(material);

		auto scene = Scene::GetScene(entity.GetSceneUUID());
		TransformComponent worldTransform = scene->GetWorldSpaceTransform(entity);

		glm::vec3 colliderSize = glm::abs(worldTransform.Scale * component.HalfSize);
		physx::PxBoxGeometry geometry = physx::PxBoxGeometry(colliderSize.x, colliderSize.y, colliderSize.z);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetPhysXActor(), geometry, *m_Material);
		m_Shape->setSimulationFilterData(actor.GetFilterData());
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !component.IsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.IsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(component.Offset, glm::vec3(0.0f)));
		m_Shape->userData = this;
	}

	BoxColliderShape::~BoxColliderShape() {}

	void BoxColliderShape::SetHalfSize(const glm::vec3& halfSize)
	{
		Ref<PhysicsActor> actor = (PhysicsActor*)m_Shape->getActor()->userData;
		Entity entity = actor->GetEntity();

		auto scene = Scene::GetScene(entity.GetSceneUUID());
		TransformComponent worldTransform = scene->GetWorldSpaceTransform(entity);

		glm::vec3 colliderSize = worldTransform.Scale * halfSize;

		physx::PxBoxGeometry geometry = physx::PxBoxGeometry(colliderSize.x, colliderSize.y, colliderSize.z);
		m_Shape->setGeometry(geometry);

		auto& component = m_Entity.GetComponent<BoxColliderComponent>();
		component.HalfSize = halfSize;
	}

	void BoxColliderShape::SetOffset(const glm::vec3& offset)
	{
		auto& component = m_Entity.GetComponent<BoxColliderComponent>();
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(offset, glm::vec3(0.0f)));
		component.Offset = offset;
	}

	void BoxColliderShape::SetTrigger(bool isTrigger)
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		auto& component = m_Entity.GetComponent<BoxColliderComponent>();
		component.IsTrigger = isTrigger;
	}

	void BoxColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void BoxColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		ANT_CORE_ASSERT(actor);
		ANT_CORE_ASSERT(m_Shape);
		actor->detachShape(*m_Shape);
	}

	SphereColliderShape::SphereColliderShape(SphereColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::Sphere, entity)
	{
		Ref<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);

		if (!material)
		{
			component.Material = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>(0.6f, 0.6f, 0.0f);
			material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);
		}

		SetMaterial(material);

		auto scene = Scene::GetScene(entity.GetSceneUUID());
		TransformComponent worldTransform = scene->GetWorldSpaceTransform(entity);
		float largestComponent = glm::max(worldTransform.Scale.x, glm::max(worldTransform.Scale.y, worldTransform.Scale.z));

		physx::PxSphereGeometry geometry = physx::PxSphereGeometry(largestComponent * component.Radius);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetPhysXActor(), geometry, *m_Material);
		m_Shape->setSimulationFilterData(actor.GetFilterData());
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !component.IsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.IsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(component.Offset, glm::vec3(0.0f)));
		m_Shape->userData = this;
	}

	SphereColliderShape::~SphereColliderShape() {}

	void SphereColliderShape::SetRadius(float radius)
	{
		Ref<PhysicsActor> actor = (PhysicsActor*)m_Shape->getActor()->userData;
		Entity entity = actor->GetEntity();

		auto scene = Scene::GetScene(entity.GetSceneUUID());
		TransformComponent worldTransform = scene->GetWorldSpaceTransform(entity);
		float largestComponent = glm::max(worldTransform.Scale.x, glm::max(worldTransform.Scale.y, worldTransform.Scale.z));

		physx::PxSphereGeometry geometry = physx::PxSphereGeometry(largestComponent * radius);
		m_Shape->setGeometry(geometry);

		auto& component = m_Entity.GetComponent<SphereColliderComponent>();
		component.Radius = radius;
	}

	void SphereColliderShape::SetOffset(const glm::vec3& offset)
	{
		auto& component = m_Entity.GetComponent<SphereColliderComponent>();
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(offset, glm::vec3(0.0f)));
		component.Offset = offset;
	}

	void SphereColliderShape::SetTrigger(bool isTrigger)
	{
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		auto& component = m_Entity.GetComponent<SphereColliderComponent>();
		component.IsTrigger = isTrigger;
	}

	void SphereColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void SphereColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		ANT_CORE_ASSERT(actor);
		ANT_CORE_ASSERT(m_Shape);
		actor->detachShape(*m_Shape);
	}

	CapsuleColliderShape::CapsuleColliderShape(CapsuleColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::Capsule, entity)
	{
		Ref<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);

		if (!material)
		{
			component.Material = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>(0.6f, 0.6f, 0.0f);
			material = AssetManager::GetAsset<PhysicsMaterial>(component.Material);
		}

		SetMaterial(material);

		auto scene = Scene::GetScene(entity.GetSceneUUID());
		TransformComponent worldTransform = scene->GetWorldSpaceTransform(entity);
		float radiusScale = glm::max(worldTransform.Scale.x, worldTransform.Scale.z);

		physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(component.Radius * radiusScale, (component.Height / 2.0f) * worldTransform.Scale.y);
		m_Shape = physx::PxRigidActorExt::createExclusiveShape(actor.GetPhysXActor(), geometry, *m_Material);
		m_Shape->setSimulationFilterData(actor.GetFilterData());
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !component.IsTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.IsTrigger);
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(component.Offset, glm::vec3(0.0f, 0.0f, physx::PxHalfPi)));
		m_Shape->userData = this;
	}

	CapsuleColliderShape::~CapsuleColliderShape() {}

	void CapsuleColliderShape::SetRadius(float radius)
	{
		Ref<PhysicsActor> actor = (PhysicsActor*)m_Shape->getActor()->userData;
		Entity entity = actor->GetEntity();

		auto scene = Scene::GetScene(entity.GetSceneUUID());
		TransformComponent worldTransform = scene->GetWorldSpaceTransform(entity);
		float radiusScale = glm::max(worldTransform.Scale.x, worldTransform.Scale.z);

		physx::PxCapsuleGeometry oldGeometry;
		m_Shape->getCapsuleGeometry(oldGeometry);

		physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(radiusScale * radius, oldGeometry.halfHeight);
		m_Shape->setGeometry(geometry);

		auto& component = m_Entity.GetComponent<CapsuleColliderComponent>();
		component.Radius = radius;
	}

	void CapsuleColliderShape::SetHeight(float height)
	{
		Ref<PhysicsActor> actor = (PhysicsActor*)m_Shape->getActor()->userData;
		Entity entity = actor->GetEntity();

		auto scene = Scene::GetScene(entity.GetSceneUUID());
		TransformComponent worldTransform = scene->GetWorldSpaceTransform(entity);

		physx::PxCapsuleGeometry oldGeometry;
		m_Shape->getCapsuleGeometry(oldGeometry);

		physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(oldGeometry.radius, (height / 2.0f) * worldTransform.Scale.y);
		m_Shape->setGeometry(geometry);

		auto& component = m_Entity.GetComponent<CapsuleColliderComponent>();
		component.Height = height;
	}

	void CapsuleColliderShape::SetOffset(const glm::vec3& offset)
	{
		auto& component = m_Entity.GetComponent<CapsuleColliderComponent>();
		component.Offset = offset;
		m_Shape->setLocalPose(PhysXUtils::ToPhysXTransform(offset, glm::vec3(0.0f)));
	}

	void CapsuleColliderShape::SetTrigger(bool isTrigger)
	{
		auto& component = m_Entity.GetComponent<CapsuleColliderComponent>();
		m_Shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !isTrigger);
		m_Shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		component.IsTrigger = isTrigger;
	}

	void CapsuleColliderShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		m_Shape->setSimulationFilterData(filterData);
	}

	void CapsuleColliderShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		ANT_CORE_ASSERT(actor);
		ANT_CORE_ASSERT(m_Shape);
		actor->detachShape(*m_Shape);
	}

	ConvexMeshShape::ConvexMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::ConvexMesh, entity, component.UseSharedShape)
	{
		ANT_PROFILE_FUNC();

		Ref<MeshColliderAsset> colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(component.ColliderAsset);
		ANT_CORE_VERIFY(colliderAsset);

		SharedShapeManager::SharedShapeData* sharedData = SharedShapeManager::FindSuitableSharedShape(ColliderType::ConvexMesh, colliderAsset);
		if (component.UseSharedShape && sharedData != nullptr && sharedData->Shapes.find(component.SubmeshIndex) != sharedData->Shapes.end())
		{
			for (auto [submeshIndex, shape] : sharedData->Shapes)
			{
				actor.GetPhysXActor().attachShape(*shape);
				m_Shapes.push_back(shape);
			}
		}
		else
		{
			Ref<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(colliderAsset->Material);

			if (!material)
			{
				material = AssetManager::GetAsset<PhysicsMaterial>(component.OverrideMaterial);
				if (!material)
				{
					component.OverrideMaterial = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>(0.6f, 0.6f, 0.0f);
					material = AssetManager::GetAsset<PhysicsMaterial>(component.OverrideMaterial);
				}
			}

			SetMaterial(material);

			const CachedColliderData& colliderData = PhysicsSystem::GetMeshCache().GetMeshData(colliderAsset);
			const auto& meshData = colliderData.SimpleColliderData;
			ANT_CORE_ASSERT(meshData.Submeshes.size() > component.SubmeshIndex);

			const SubmeshColliderData& submesh = meshData.Submeshes[component.SubmeshIndex];
			glm::vec3 submeshTranslation;
			glm::quat submeshRotation;
			glm::vec3 submeshScale;
			Math::DecomposeTransform(submesh.Transform, submeshTranslation, submeshRotation, submeshScale);

			physx::PxDefaultMemoryInputData input(submesh.ColliderData.As<physx::PxU8>(), submesh.ColliderData.Size);
			physx::PxConvexMesh* convexMesh = PhysXInternal::GetPhysXSDK().createConvexMesh(input);

			if (convexMesh)
			{
				auto scene = Scene::GetScene(entity.GetSceneUUID());
				TransformComponent worldTransform = scene->GetWorldSpaceTransform(entity);

				physx::PxConvexMeshGeometry convexGeometry = physx::PxConvexMeshGeometry(convexMesh, physx::PxMeshScale(PhysXUtils::ToPhysXVector(submeshScale * worldTransform.Scale)));
				convexGeometry.meshFlags = physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS;

				physx::PxShape* shape = PhysXInternal::GetPhysXSDK().createShape(convexGeometry, *m_Material, !component.UseSharedShape);
				shape->setSimulationFilterData(actor.GetFilterData());
				shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, colliderAsset->CollisionComplexity != ECollisionComplexity::UseComplexAsSimple && !component.IsTrigger);
				shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, component.IsTrigger);
				shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, colliderAsset->CollisionComplexity == ECollisionComplexity::UseSimpleAsComplex);
				shape->userData = this;

				if (component.UseSharedShape)
				{
					if (sharedData != nullptr)
					{
						// Found suitable shared data, but our submesh doesn't exist in it
						sharedData->Shapes[component.SubmeshIndex] = shape;
					}
					else
					{
						// Failed to find suitable shape data, create a new one
						sharedData = SharedShapeManager::CreateSharedShapeData(ColliderType::ConvexMesh, colliderAsset->ColliderMesh);
						sharedData->Usage = colliderAsset->CollisionComplexity;
						sharedData->Shapes[component.SubmeshIndex] = shape;
					}
				}

				actor.GetPhysXActor().attachShape(*shape);

				m_Shapes.push_back(shape);

				shape->release();
				convexMesh->release();
			}
			else
			{
				ANT_CORE_ERROR_TAG("Physics", "Failed to create Convex shape!");
				m_Material->release();
			}
		}
	}

	ConvexMeshShape::~ConvexMeshShape() {}

	void ConvexMeshShape::SetTrigger(bool isTrigger)
	{
		auto& component = m_Entity.GetComponent<MeshColliderComponent>();
		if (component.UseSharedShape)
			return;

		Ref<MeshColliderAsset> colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(component.ColliderAsset);

		for (auto shape : m_Shapes)
		{
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, colliderAsset->CollisionComplexity != ECollisionComplexity::UseComplexAsSimple && !isTrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, isTrigger);
		}

		component.IsTrigger = isTrigger;
	}

	void ConvexMeshShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		auto& component = m_Entity.GetComponent<MeshColliderComponent>();
		if (component.UseSharedShape)
			return;

		for (auto shape : m_Shapes)
			shape->setSimulationFilterData(filterData);
	}

	void ConvexMeshShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		ANT_CORE_ASSERT(actor);

		uint32_t referenceCount = m_Shapes[0]->getReferenceCount();

		for (auto shape : m_Shapes)
		{
			ANT_CORE_ASSERT(shape);
			actor->detachShape(*shape);
		}

		const auto& component = m_Entity.GetComponent<MeshColliderComponent>();

		// Reference count was 1, meaning the shape has now been destroyed, so clear it from the map
		if (referenceCount == 1)
		{
			Ref<MeshColliderAsset> colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(component.ColliderAsset);
			SharedShapeManager::RemoveSharedShapeData(ColliderType::ConvexMesh, colliderAsset, true, component.SubmeshIndex);
		}

		m_Shapes.clear();
	}

	TriangleMeshShape::TriangleMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity)
		: ColliderShape(ColliderType::TriangleMesh, entity, component.UseSharedShape)
	{
		ANT_PROFILE_FUNC();

		Ref<MeshColliderAsset> colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(component.ColliderAsset);
		ANT_CORE_VERIFY(colliderAsset);

		SharedShapeManager::SharedShapeData* sharedData = SharedShapeManager::FindSuitableSharedShape(ColliderType::TriangleMesh, colliderAsset);
		if (component.UseSharedShape && sharedData != nullptr)
		{
			for (auto [submeshIndex, shape] : sharedData->Shapes)
			{
				actor.GetPhysXActor().attachShape(*shape);
				m_Shapes.push_back(shape);
			}
		}
		else
		{
			Ref<PhysicsMaterial> material = AssetManager::GetAsset<PhysicsMaterial>(colliderAsset->Material);

			if (!material)
			{
				material = AssetManager::GetAsset<PhysicsMaterial>(component.OverrideMaterial);
				if (!material)
				{
					component.OverrideMaterial = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>(0.6f, 0.6f, 0.0f);
					material = AssetManager::GetAsset<PhysicsMaterial>(component.OverrideMaterial);
				}
			}

			SetMaterial(material);

			const CachedColliderData& colliderData = PhysicsSystem::GetMeshCache().GetMeshData(colliderAsset);
			const auto& meshData = colliderData.ComplexColliderData;

			auto scene = Scene::GetScene(entity.GetSceneUUID());
			TransformComponent worldTransform = scene->GetWorldSpaceTransform(entity);

			for (size_t i = 0; i < meshData.Submeshes.size(); i++)
			{
				const SubmeshColliderData& submeshData = meshData.Submeshes[i];

				glm::vec3 submeshTranslation;
				glm::quat submeshRotation;
				glm::vec3 submeshScale;
				Math::DecomposeTransform(submeshData.Transform, submeshTranslation, submeshRotation, submeshScale);

				physx::PxDefaultMemoryInputData input(submeshData.ColliderData.As<physx::PxU8>(), submeshData.ColliderData.Size);
				physx::PxTriangleMesh* trimesh = PhysXInternal::GetPhysXSDK().createTriangleMesh(input);

				if (trimesh)
				{
					physx::PxTriangleMeshGeometry triangleGeometry = physx::PxTriangleMeshGeometry(trimesh, physx::PxMeshScale(PhysXUtils::ToPhysXVector(submeshScale * worldTransform.Scale)));

					physx::PxShape* shape = PhysXInternal::GetPhysXSDK().createShape(triangleGeometry, *m_Material, !component.UseSharedShape);
					shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, colliderAsset->CollisionComplexity == ECollisionComplexity::UseComplexAsSimple);
					shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
					shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, colliderAsset->CollisionComplexity != ECollisionComplexity::UseSimpleAsComplex);
					shape->setSimulationFilterData(actor.GetFilterData());
					shape->setLocalPose(PhysXUtils::ToPhysXTransform(submeshTranslation, submeshRotation));
					shape->userData = this;

					if (component.UseSharedShape)
					{
						if (sharedData != nullptr)
						{
							sharedData->Shapes[(uint32_t)i] = shape;
						}
						else
						{
							sharedData = SharedShapeManager::CreateSharedShapeData(ColliderType::TriangleMesh, colliderAsset->ColliderMesh);
							sharedData->Usage = colliderAsset->CollisionComplexity;
							sharedData->Shapes[(uint32_t)i] = shape;
						}
					}

					actor.GetPhysXActor().attachShape(*shape);

					m_Shapes.push_back(shape);

					shape->release();
					trimesh->release();
				}
				else
				{
					ANT_CORE_ERROR_TAG("Physics", "Failed to create Triangle shape!");
					m_Material->release();
				}
			}
		}
	}

	TriangleMeshShape::~TriangleMeshShape() {}

	void TriangleMeshShape::SetTrigger(bool isTrigger)
	{
		// NOTE(Peter): Triangle mesh colliders cannot be triggers!
	}

	void TriangleMeshShape::SetFilterData(const physx::PxFilterData& filterData)
	{
		const auto& component = m_Entity.GetComponent<MeshColliderComponent>();
		if (component.UseSharedShape)
			return;

		for (auto shape : m_Shapes)
			shape->setSimulationFilterData(filterData);
	}

	void TriangleMeshShape::DetachFromActor(physx::PxRigidActor* actor)
	{
		ANT_CORE_ASSERT(actor);

		uint32_t referenceCount = m_Shapes[0]->getReferenceCount();

		for (auto shape : m_Shapes)
		{
			ANT_CORE_ASSERT(shape);
			actor->detachShape(*shape);
		}

		const auto& component = m_Entity.GetComponent<MeshColliderComponent>();

		// Reference count was 1, meaning the shape has now been destroyed, so clear it from the map
		if (referenceCount == 1)
		{
			Ref<MeshColliderAsset> colliderAsset = AssetManager::GetAsset<MeshColliderAsset>(component.ColliderAsset);
			SharedShapeManager::RemoveSharedShapeData(ColliderType::TriangleMesh, colliderAsset);
		}

		m_Shapes.clear();
	}

	SharedShapeManager::SharedShapeData* SharedShapeManager::CreateSharedShapeData(ColliderType colliderType, AssetHandle colliderHandle)
	{
		SharedShapeData* sharedData = anew SharedShapeData();
		s_SharedShapes[colliderType][colliderHandle].push_back(sharedData);
		return sharedData;
	}

	SharedShapeManager::SharedShapeData* SharedShapeManager::FindSuitableSharedShape(ColliderType colliderType, const Ref<MeshColliderAsset>& collider, const bool checkSubmeshIndex, const uint32_t submeshIndex)
	{
		if (s_SharedShapes.find(colliderType) == s_SharedShapes.end())
			return nullptr;

		auto& colliderShapeMap = s_SharedShapes.at(colliderType);

		if (colliderShapeMap.find(collider->ColliderMesh) == colliderShapeMap.end())
			return nullptr;

		for (SharedShapeData* shapeData : colliderShapeMap.at(collider->ColliderMesh))
		{
			if (shapeData->Usage != collider->CollisionComplexity)
				continue;

			if (checkSubmeshIndex && shapeData->Shapes.find(submeshIndex) == shapeData->Shapes.end())
				continue;

			return shapeData;
		}

		return nullptr;
	}

	void SharedShapeManager::RemoveSharedShapeData(ColliderType colliderType, const Ref<MeshColliderAsset>& collider, const bool checkSubmeshIndex, const uint32_t submeshIndex)
	{
		auto* shapeData = FindSuitableSharedShape(colliderType, collider, checkSubmeshIndex, submeshIndex);

		if (shapeData == nullptr)
			return;

		auto& sharedShapes = s_SharedShapes[colliderType][collider->ColliderMesh];
		sharedShapes.erase(std::find(sharedShapes.begin(), sharedShapes.end(), shapeData));
		adelete shapeData;
	}

	void SharedShapeManager::ClearSharedShapes()
	{
		for (auto& [colliderType, colliderShapeMap] : s_SharedShapes)
		{
			for (auto& [colliderHandle, sharedShapes] : colliderShapeMap)
			{
				for (auto sharedShapeData : sharedShapes)
					adelete sharedShapeData;

				sharedShapes.clear();
			}

			colliderShapeMap.clear();
		}

		s_SharedShapes.clear();
	}

	SharedShapeManager::SharedShapeMap SharedShapeManager::s_SharedShapes;
}