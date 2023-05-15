#include "antpch.h"
#include "ScriptWrappers.h"

#include "Ant/Core/Math/Noise.h"

#include "Ant/Scene/Scene.h"
#include "Ant/Scene/Entity.h"
#include "Ant/Physics/PhysicsUtil.h"
#include "Ant/Physics/PXPhysicsWrappers.h"
#include "Ant/Physics/PhysicsActor.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/common.hpp>

#include <mono/jit/jit.h>

#include <box2d/box2d.h>

#include <PhysX/PxPhysicsAPI.h>

namespace Ant {
	extern std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
	extern std::unordered_map<MonoType*, std::function<void(Entity&)>> s_CreateComponentFuncs;
}

namespace Ant { namespace Script {


	////////////////////////////////////////////////////////////////
	// Math ////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	float Ant_Noise_PerlinNoise(float x, float y)
	{
		return Noise::PerlinNoise(x, y);
	}

	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// Input ///////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	bool Ant_Input_IsKeyPressed(KeyCode key)
	{
		return Input::IsKeyPressed(key);
	}

	bool Ant_Input_IsMouseButtonPressed(MouseCode button)
	{
		return Input::IsMouseButtonPressed(button);
	}

	void Ant_Input_GetMousePosition(glm::vec2* outPosition)
	{
		auto [x, y] = Input::GetMousePosition();
		*outPosition = { x, y };
	}

	void Ant_Input_SetCursorMode(CursorMode mode)
	{
		Input::SetCursorMode(mode);
	}

	CursorMode Ant_Input_GetCursorMode()
	{
		return Input::GetCursorMode();
	}

	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// Physics //////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	bool Ant_Physics_Raycast(glm::vec3* origin, glm::vec3* direction, float maxDistance, RaycastHit* hit)
	{
		return PXPhysicsWrappers::Raycast(*origin, *direction, maxDistance, hit);
	}

	// Helper function for the Overlap functions below
	static void AddCollidersToArray(MonoArray* array, const std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& hits, uint32_t count, uint32_t arrayLength)
	{
		uint32_t arrayIndex = 0;
		for (uint32_t i = 0; i < count; i++)
		{
			Entity& entity = *(Entity*)hits[i].actor->userData;

			if (entity.HasComponent<BoxColliderComponent>() && arrayIndex < arrayLength)
			{
				auto& boxCollider = entity.GetComponent<BoxColliderComponent>();

				void* data[] = {
					&entity.GetUUID(),
					&boxCollider.IsTrigger,
					&boxCollider.Size,
					&boxCollider.Offset
				};

				MonoObject* obj = ScriptEngine::Construct("Ant.BoxCollider:.ctor(ulong,bool,Vector3,Vector3)", true, data);
				mono_array_set(array, MonoObject*, arrayIndex++, obj);
			}

			if (entity.HasComponent<SphereColliderComponent>() && arrayIndex < arrayLength)
			{
				auto& sphereCollider = entity.GetComponent<SphereColliderComponent>();

				void* data[] = {
					&entity.GetUUID(),
					&sphereCollider.IsTrigger,
					&sphereCollider.Radius
				};

				MonoObject* obj = ScriptEngine::Construct("Ant.SphereCollider:.ctor(ulong,bool,single)", true, data);
				mono_array_set(array, MonoObject*, arrayIndex++, obj);
			}

			if (entity.HasComponent<CapsuleColliderComponent>() && arrayIndex < arrayLength)
			{
				auto& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();

				void* data[] = {
					&entity.GetUUID(),
					&capsuleCollider.IsTrigger,
					&capsuleCollider.Radius,
					&capsuleCollider.Height
				};

				MonoObject* obj = ScriptEngine::Construct("Ant.CapsuleCollider:.ctor(ulong,bool,single,single)", true, data);
				mono_array_set(array, MonoObject*, arrayIndex++, obj);
			}

			if (entity.HasComponent<MeshColliderComponent>() && arrayIndex < arrayLength)
			{
				auto& meshCollider = entity.GetComponent<MeshColliderComponent>();

				Ref<Mesh>* mesh = new Ref<Mesh>(meshCollider.CollisionMesh);
				void* data[] = {
					&entity.GetUUID(),
					&meshCollider.IsTrigger,
					&mesh
				};

				MonoObject* obj = ScriptEngine::Construct("Ant.MeshCollider:.ctor(ulong,bool,intptr)", true, data);
				mono_array_set(array, MonoObject*, arrayIndex++, obj);
			}
		}
	}

	static std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> s_OverlapBuffer;

	MonoArray* Ant_Physics_OverlapBox(glm::vec3* origin, glm::vec3* halfSize)
	{
		MonoArray* outColliders = nullptr;
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint32_t count;
		if (PXPhysicsWrappers::OverlapBox(*origin, *halfSize, s_OverlapBuffer, &count))
		{
			outColliders = mono_array_new(mono_domain_get(), ScriptEngine::GetCoreClass("Ant.Collider"), count);
			AddCollidersToArray(outColliders, s_OverlapBuffer, count, count);
		}

		return outColliders;
	}

	MonoArray* Ant_Physics_OverlapCapsule(glm::vec3* origin, float radius, float halfHeight)
	{
		MonoArray* outColliders = nullptr;
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint32_t count;
		if (PXPhysicsWrappers::OverlapCapsule(*origin, radius, halfHeight, s_OverlapBuffer, &count))
		{
			outColliders = mono_array_new(mono_domain_get(), ScriptEngine::GetCoreClass("Ant.Collider"), count);
			AddCollidersToArray(outColliders, s_OverlapBuffer, count, count);
		}

		return outColliders;
	}

	MonoArray* Ant_Physics_OverlapSphere(glm::vec3* origin, float radius)
	{
		MonoArray* outColliders = nullptr;
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint32_t count;
		if (PXPhysicsWrappers::OverlapSphere(*origin, radius, s_OverlapBuffer, &count))
		{
			outColliders = mono_array_new(mono_domain_get(), ScriptEngine::GetCoreClass("Ant.Collider"), count);
			AddCollidersToArray(outColliders, s_OverlapBuffer, count, count);
		}

		return outColliders;
	}

	int32_t Ant_Physics_OverlapBoxNonAlloc(glm::vec3* origin, glm::vec3* halfSize, MonoArray* outColliders)
	{
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint64_t arrayLength = mono_array_length(outColliders);

		uint32_t count;
		if (PXPhysicsWrappers::OverlapBox(*origin, *halfSize, s_OverlapBuffer, &count))
		{
			if (count > arrayLength)
				count = arrayLength;

			AddCollidersToArray(outColliders, s_OverlapBuffer, count, arrayLength);
		}

		return count;
	}

	int32_t Ant_Physics_OverlapCapsuleNonAlloc(glm::vec3* origin, float radius, float halfHeight, MonoArray* outColliders)
	{
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint64_t arrayLength = mono_array_length(outColliders);
		uint32_t count;
		if (PXPhysicsWrappers::OverlapCapsule(*origin, radius, halfHeight, s_OverlapBuffer, &count))
		{
			if (count > arrayLength)
				count = arrayLength;

			AddCollidersToArray(outColliders, s_OverlapBuffer, count, arrayLength);
		}

		return count;
	}

	int32_t Ant_Physics_OverlapSphereNonAlloc(glm::vec3* origin, float radius, MonoArray* outColliders)
	{
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint64_t arrayLength = mono_array_length(outColliders);

		uint32_t count;
		if (PXPhysicsWrappers::OverlapSphere(*origin, radius, s_OverlapBuffer, &count))
		{
			if (count > arrayLength)
				count = arrayLength;

			AddCollidersToArray(outColliders, s_OverlapBuffer, count, arrayLength);
		}

		return count;
	}

	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	// Entity //////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////

	void Ant_Entity_CreateComponent(uint64_t entityID, void* type)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		s_CreateComponentFuncs[monoType](entity);
	}

	bool Ant_Entity_HasComponent(uint64_t entityID, void* type)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		bool result = s_HasComponentFuncs[monoType](entity);
		return result;
	}

	uint64_t Ant_Entity_FindEntityByTag(MonoString* tag)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");

		Entity entity = scene->FindEntityByTag(mono_string_to_utf8(tag));
		if (entity)
			return entity.GetComponent<IDComponent>().ID;

		return 0;
	}

	void Ant_TransformComponent_GetTransform(uint64_t entityID, TransformComponent* outTransform)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		*outTransform = entity.GetComponent<TransformComponent>();
	}

	void Ant_TransformComponent_SetTransform(uint64_t entityID, TransformComponent* inTransform)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		entity.GetComponent<TransformComponent>() = *inTransform;
	}

	void Ant_TransformComponent_GetTranslation(uint64_t entityID, glm::vec3* outTranslation)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		*outTranslation = entity.GetComponent<TransformComponent>().Translation;
	}

	void Ant_TransformComponent_SetTranslation(uint64_t entityID, glm::vec3* inTranslation)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		entity.GetComponent<TransformComponent>().Translation = *inTranslation;
	}

	void Ant_TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		*outRotation = entity.GetComponent<TransformComponent>().Rotation;
	}

	void Ant_TransformComponent_SetRotation(uint64_t entityID, glm::vec3* inRotation)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		entity.GetComponent<TransformComponent>().Rotation = *inRotation;
	}

	void Ant_TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		*outScale = entity.GetComponent<TransformComponent>().Scale;
	}

	void Ant_TransformComponent_SetScale(uint64_t entityID, glm::vec3* inScale)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		entity.GetComponent<TransformComponent>().Scale = *inScale;
	}

	void* Ant_MeshComponent_GetMesh(uint64_t entityID)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		auto& meshComponent = entity.GetComponent<MeshComponent>();
		return new Ref<Mesh>(meshComponent.Mesh);
	}

	void Ant_MeshComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		auto& meshComponent = entity.GetComponent<MeshComponent>();
		meshComponent.Mesh = inMesh ? *inMesh : nullptr;
	}

	void Ant_RigidBody2DComponent_ApplyLinearImpulse(uint64_t entityID, glm::vec2* impulse, glm::vec2* offset, bool wake)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBody2DComponent>());
		auto& component = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)component.RuntimeBody;
		body->ApplyLinearImpulse(*(const b2Vec2*)impulse, body->GetWorldCenter() + *(const b2Vec2*)offset, wake);
	}

	void Ant_RigidBody2DComponent_GetLinearVelocity(uint64_t entityID, glm::vec2* outVelocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBody2DComponent>());
		auto& component = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)component.RuntimeBody;
		const auto& velocity = body->GetLinearVelocity();
		ANT_CORE_ASSERT(outVelocity);
		*outVelocity = { velocity.x, velocity.y };
	}

	void Ant_RigidBody2DComponent_SetLinearVelocity(uint64_t entityID, glm::vec2* velocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBody2DComponent>());
		auto& component = entity.GetComponent<RigidBody2DComponent>();
		b2Body* body = (b2Body*)component.RuntimeBody;
		ANT_CORE_ASSERT(velocity);
		body->SetLinearVelocity({ velocity->x, velocity->y });
	}

	RigidBodyComponent::Type Ant_RigidBodyComponent_GetBodyType(uint64_t entityID)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();
		return component.BodyType;
	}

	void Ant_RigidBodyComponent_AddForce(uint64_t entityID, glm::vec3* force, ForceMode forceMode)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();

		if (component.IsKinematic)
		{
			ANT_CORE_WARN("Cannot add a force to a kinematic actor! EntityID({0})", entityID);
			return;
		}

		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->AddForce(*force, forceMode);
	}

	void Ant_RigidBodyComponent_AddTorque(uint64_t entityID, glm::vec3* torque, ForceMode forceMode)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();

		if (component.IsKinematic)
		{
			ANT_CORE_WARN("Cannot add torque to a kinematic actor! EntityID({0})", entityID);
			return;
		}

		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->AddTorque(*torque, forceMode);
	}

	void Ant_RigidBodyComponent_GetLinearVelocity(uint64_t entityID, glm::vec3* outVelocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ANT_CORE_ASSERT(outVelocity);
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		*outVelocity = actor->GetLinearVelocity();
	}

	void Ant_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, glm::vec3* velocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ANT_CORE_ASSERT(velocity);
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->SetLinearVelocity(*velocity);
	}

	void Ant_RigidBodyComponent_GetAngularVelocity(uint64_t entityID, glm::vec3* outVelocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ANT_CORE_ASSERT(outVelocity);
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		*outVelocity = actor->GetAngularVelocity();
	}

	void Ant_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, glm::vec3* velocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ANT_CORE_ASSERT(velocity);
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->SetAngularVelocity(*velocity);
	}

	void Ant_RigidBodyComponent_Rotate(uint64_t entityID, glm::vec3* rotation)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ANT_CORE_ASSERT(rotation);
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->Rotate(*rotation);
	}

	uint32_t Ant_RigidBodyComponent_GetLayer(uint64_t entityID)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();
		return component.Layer;
	}

	float Ant_RigidBodyComponent_GetMass(uint64_t entityID)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();

		Ref<PhysicsActor>& actor = Physics::GetActorForEntity(entity);
		return actor->GetMass();
	}

	void Ant_RigidBodyComponent_SetMass(uint64_t entityID, float mass)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ANT_CORE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ANT_CORE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ANT_CORE_ASSERT(entity.HasComponent<RigidBodyComponent>());
		auto& component = entity.GetComponent<RigidBodyComponent>();

		Ref<PhysicsActor>& actor = Physics::GetActorForEntity(entity);
		actor->SetMass(mass);
	}

	Ref<Mesh>* Ant_Mesh_Constructor(MonoString* filepath)
	{
		return new Ref<Mesh>(new Mesh(mono_string_to_utf8(filepath)));
	}

	void Ant_Mesh_Destructor(Ref<Mesh>* _this)
	{
		Ref<Mesh>* instance = (Ref<Mesh>*)_this;
		delete _this;
	}

	Ref<Material>* Ant_Mesh_GetMaterial(Ref<Mesh>* inMesh)
	{
		Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
		const auto& materials = mesh->GetMaterials();
		return new Ref<Material>(materials[0]);
	}

	Ref<Material>* Ant_Mesh_GetMaterialByIndex(Ref<Mesh>* inMesh, int index)
	{
		Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
		const auto& materials = mesh->GetMaterials();

		ANT_CORE_ASSERT(index < materials.size());
		return new Ref<Material>(materials[index]);
	}

	int Ant_Mesh_GetMaterialCount(Ref<Mesh>* inMesh)
	{
		Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
		const auto& materials = mesh->GetMaterials();
		return materials.size();
	}

	void* Ant_Texture2D_Constructor(uint32_t width, uint32_t height)
	{
		auto result = Texture2D::Create(ImageFormat::RGBA, width, height);
		return new Ref<Texture2D>(result);
	}

	void Ant_Texture2D_Destructor(Ref<Texture2D>* _this)
	{
		delete _this;
	}

	void Ant_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* inData, int32_t count)
	{
		Ref<Texture2D>& instance = *_this;

		uint32_t dataSize = count * sizeof(glm::vec4) / 4;

		instance->Lock();
		Buffer buffer = instance->GetWriteableBuffer();
		ANT_CORE_ASSERT(dataSize <= buffer.Size);
		// Convert RGBA32F color to RGBA8
		uint8_t* pixels = (uint8_t*)buffer.Data;
		uint32_t index = 0;
		for (int i = 0; i < instance->GetWidth() * instance->GetHeight(); i++)
		{
			glm::vec4& value = mono_array_get(inData, glm::vec4, i);
			*pixels++ = (uint32_t)(value.x * 255.0f);
			*pixels++ = (uint32_t)(value.y * 255.0f);
			*pixels++ = (uint32_t)(value.z * 255.0f);
			*pixels++ = (uint32_t)(value.w * 255.0f);
		}

		instance->Unlock();
	}

	void Ant_Material_Destructor(Ref<Material>* _this)
	{
		delete _this;
	}

	void Ant_Material_SetFloat(Ref<Material>* _this, MonoString* uniform, float value)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), value);
	}

	void Ant_Material_SetTexture(Ref<Material>* _this, MonoString* uniform, Ref<Texture2D>* texture)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *texture);
	}

	void Ant_MaterialInstance_Destructor(Ref<Material>* _this)
	{
		delete _this;
	}

	void Ant_MaterialInstance_SetFloat(Ref<Material>* _this, MonoString* uniform, float value)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), value);
	}

	void Ant_MaterialInstance_SetVector3(Ref<Material>* _this, MonoString* uniform, glm::vec3* value)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *value);
	}

	void Ant_MaterialInstance_SetVector4(Ref<Material>* _this, MonoString* uniform, glm::vec4* value)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *value);
	}

	void Ant_MaterialInstance_SetTexture(Ref<Material>* _this, MonoString* uniform, Ref<Texture2D>* texture)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *texture);
	}

	void* Ant_MeshFactory_CreatePlane(float width, float height)
	{
		// TODO: Implement properly with MeshFactory class!
		return new Ref<Mesh>(new Mesh("assets/models/Plane1m.obj"));
	}

	////////////////////////////////////////////////////////////////

} }