#pragma once

#include "PhysXUtils.h"
#include "Ant/Scene/Entity.h"

#include "CookingFactory.h"

namespace Ant {

	// TODO: Heightfield, Plane
	enum class ColliderType
	{
		Box, Sphere, Capsule, ConvexMesh, TriangleMesh
	};

	class ColliderShape : public RefCounted
	{
	protected:
		ColliderShape(ColliderType type, Entity entity, bool isShared = false)
			: m_Type(type), m_Material(nullptr), m_IsShared(isShared), m_Entity(entity) { }

	public:
		virtual ~ColliderShape()
		{
			Release();
		}

		void Release()
		{
			if (m_IsShared && m_Material == nullptr)
				return;

			ANT_CORE_ASSERT(m_Material);
			m_Material->release();
		}

		void SetMaterial(Ref<PhysicsMaterial> material);

		ColliderType GetType() const { return m_Type; }

		virtual const glm::vec3& GetOffset() const = 0;
		virtual void SetOffset(const glm::vec3& offset) = 0;

		virtual bool IsTrigger() const = 0;
		virtual void SetTrigger(bool isTrigger) = 0;

		virtual void SetFilterData(const physx::PxFilterData& filterData) = 0;

		virtual void DetachFromActor(physx::PxRigidActor* actor) = 0;

		virtual const char* GetShapeName() const = 0;

		physx::PxMaterial& GetMaterial() const { return *m_Material; }
		bool IsShared() const { return m_IsShared; }

		virtual bool IsValid() const { return m_Material != nullptr; }

	protected:
		ColliderType m_Type;
		bool m_IsShared = false;

		Entity m_Entity;
		physx::PxMaterial* m_Material;
	};

	class PhysicsActor;

	class BoxColliderShape : public ColliderShape
	{
	public:
		BoxColliderShape(BoxColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~BoxColliderShape();

		const glm::vec3& GetHalfSize() const { return m_Entity.GetComponent<BoxColliderComponent>().HalfSize; }
		void SetHalfSize(const glm::vec3& halfSize);

		const glm::vec3& GetOffset() const override { return m_Entity.GetComponent<BoxColliderComponent>().Offset; }
		void SetOffset(const glm::vec3& offset) override;

		virtual bool IsTrigger() const override { return m_Entity.GetComponent<BoxColliderComponent>().IsTrigger; }
		virtual void SetTrigger(bool isTrigger) override;

		virtual void SetFilterData(const physx::PxFilterData& filterData) override;

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

		virtual const char* GetShapeName() const override { return "BoxCollider"; }
		virtual bool IsValid() const override { return ColliderShape::IsValid() && m_Shape != nullptr; }

		static ColliderType GetStaticType() { return ColliderType::Box; }

	private:
		physx::PxShape* m_Shape;
	};

	class SphereColliderShape : public ColliderShape
	{
	public:
		SphereColliderShape(SphereColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~SphereColliderShape();

		float GetRadius() const { return m_Entity.GetComponent<SphereColliderComponent>().Radius; }
		void SetRadius(float radius);

		const glm::vec3& GetOffset() const override { return m_Entity.GetComponent<SphereColliderComponent>().Offset; }
		void SetOffset(const glm::vec3& offset) override;

		virtual bool IsTrigger() const override { return m_Entity.GetComponent<SphereColliderComponent>().IsTrigger; }
		virtual void SetTrigger(bool isTrigger) override;

		virtual void SetFilterData(const physx::PxFilterData& filterData) override;

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

		virtual const char* GetShapeName() const override { return "SphereCollider"; }
		virtual bool IsValid() const override { return ColliderShape::IsValid() && m_Shape != nullptr; }

		static ColliderType GetStaticType() { return ColliderType::Sphere; }

	private:
		physx::PxShape* m_Shape;
	};

	class CapsuleColliderShape : public ColliderShape
	{
	public:
		CapsuleColliderShape(CapsuleColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~CapsuleColliderShape();

		float GetRadius() const { return m_Entity.GetComponent<CapsuleColliderComponent>().Radius; }
		void SetRadius(float radius);

		float GetHeight() const { return m_Entity.GetComponent<CapsuleColliderComponent>().Height; }
		void SetHeight(float height);

		const glm::vec3& GetOffset() const override { return m_Entity.GetComponent<CapsuleColliderComponent>().Offset; }
		void SetOffset(const glm::vec3& offset) override;

		virtual bool IsTrigger() const override { return m_Entity.GetComponent<CapsuleColliderComponent>().IsTrigger; }
		virtual void SetTrigger(bool isTrigger) override;

		virtual void SetFilterData(const physx::PxFilterData& filterData) override;

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

		virtual const char* GetShapeName() const override { return "CapsuleCollider"; }
		virtual bool IsValid() const override { return ColliderShape::IsValid() && m_Shape != nullptr; }

		static ColliderType GetStaticType() { return ColliderType::Capsule; }

	private:
		physx::PxShape* m_Shape;
	};

	class ConvexMeshShape : public ColliderShape
	{
	public:
		ConvexMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~ConvexMeshShape();

		AssetHandle GetColliderHandle() const { return m_Entity.GetComponent<MeshColliderComponent>().ColliderAsset; }

		virtual const glm::vec3& GetOffset() const
		{
			static glm::vec3 defaultOffset = glm::vec3(0.0f);
			return defaultOffset;
		}
		virtual void SetOffset(const glm::vec3& offset) {}

		virtual bool IsTrigger() const override { return m_Entity.GetComponent<MeshColliderComponent>().IsTrigger; }
		virtual void SetTrigger(bool isTrigger) override;

		virtual void SetFilterData(const physx::PxFilterData& filterData) override;

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

		virtual const char* GetShapeName() const override { return "ConvexMeshCollider"; }
		virtual bool IsValid() const override { return ColliderShape::IsValid() && !m_Shapes.empty(); }

		static ColliderType GetStaticType() { return ColliderType::ConvexMesh; }

	private:
		std::vector<physx::PxShape*> m_Shapes;
	};

	class TriangleMeshShape : public ColliderShape
	{
	public:
		TriangleMeshShape(MeshColliderComponent& component, const PhysicsActor& actor, Entity entity);
		~TriangleMeshShape();

		AssetHandle GetColliderHandle() const { return m_Entity.GetComponent<MeshColliderComponent>().ColliderAsset; }

		virtual const glm::vec3& GetOffset() const
		{
			static glm::vec3 defaultOffset = glm::vec3(0.0f);
			return defaultOffset;
		}
		virtual void SetOffset(const glm::vec3& offset) {}

		virtual bool IsTrigger() const override { return m_Entity.GetComponent<MeshColliderComponent>().IsTrigger; }
		virtual void SetTrigger(bool isTrigger) override;

		virtual void SetFilterData(const physx::PxFilterData& filterData) override;

		virtual void DetachFromActor(physx::PxRigidActor* actor) override;

		virtual const char* GetShapeName() const override { return "TriangleMeshCollider"; }
		virtual bool IsValid() const override { return ColliderShape::IsValid() && !m_Shapes.empty(); }

		static ColliderType GetStaticType() { return ColliderType::TriangleMesh; }

	private:
		std::vector<physx::PxShape*> m_Shapes;
	};

	class SharedShapeManager
	{
	public:
		struct SharedShapeData
		{
			ECollisionComplexity Usage = ECollisionComplexity::Default;
			std::unordered_map<uint32_t, physx::PxShape*> Shapes;
		};

		using SharedShapeMap = std::unordered_map<ColliderType, std::unordered_map<AssetHandle, std::vector<SharedShapeData*>>>;

	public:
		static SharedShapeData* CreateSharedShapeData(ColliderType colliderType, AssetHandle colliderHandle);
		static SharedShapeData* FindSuitableSharedShape(ColliderType colliderType, const Ref<MeshColliderAsset>& collider, const bool checkSubmeshIndex = false, const uint32_t submeshIndex = 0);
		static void RemoveSharedShapeData(ColliderType colliderType, const Ref<MeshColliderAsset>& collider, const bool checkSubmeshIndex = false, const uint32_t submeshIndex = 0);
		static void ClearSharedShapes();

	private:
		static SharedShapeMap s_SharedShapes;
	};
}
