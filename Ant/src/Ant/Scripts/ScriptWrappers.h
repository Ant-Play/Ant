#pragma once

#include "ScriptEngine.h"
#include "Ant/Core/Inputs.h"
#include "Ant/Physics/Physics.h"

#include <glm/glm.hpp>

extern "C" {
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}

namespace Ant{ namespace Script{

	// Math
	float Ant_Noise_PerlinNoise(float x, float y);

	// Input
	bool Ant_Input_IsKeyPressed(KeyCode key);
	bool Ant_Input_IsMouseButtonPressed(MouseCode button);
	void Ant_Input_GetMousePosition(glm::vec2* outPosition);
	void Ant_Input_SetCursorMode(CursorMode mode);
	CursorMode Ant_Input_GetCursorMode();

	// Physics
	bool Ant_Physics_Raycast(glm::vec3* origin, glm::vec3* direction, float maxDistance, RaycastHit* hit);
	MonoArray* Ant_Physics_OverlapBox(glm::vec3* origin, glm::vec3* halfSize);
	MonoArray* Ant_Physics_OverlapCapsule(glm::vec3* origin, float radius, float halfHeight);
	MonoArray* Ant_Physics_OverlapSphere(glm::vec3* origin, float radius);
	int32_t Ant_Physics_OverlapBoxNonAlloc(glm::vec3* origin, glm::vec3* halfSize, MonoArray* outColliders);
	int32_t Ant_Physics_OverlapCapsuleNonAlloc(glm::vec3* origin, float radius, float halfHeight, MonoArray* outColliders);
	int32_t Ant_Physics_OverlapSphereNonAlloc(glm::vec3* origin, float radius, MonoArray* outColliders);

	// Entity
	void Ant_Entity_CreateComponent(uint64_t entityID, void* type);
	bool Ant_Entity_HasComponent(uint64_t entityID, void* type);
	uint64_t Ant_Entity_FindEntityByTag(MonoString* tag);

	void Ant_TransformComponent_GetTransform(uint64_t entityID, TransformComponent* outTransform);
	void Ant_TransformComponent_SetTransform(uint64_t entityID, TransformComponent* inTransform);
	void Ant_TransformComponent_GetTranslation(uint64_t entityID, glm::vec3* outTranslation);
	void Ant_TransformComponent_SetTranslation(uint64_t entityID, glm::vec3* inTranslation);
	void Ant_TransformComponent_GetRotation(uint64_t entityID, glm::vec3* outRotation);
	void Ant_TransformComponent_SetRotation(uint64_t entityID, glm::vec3* inRotation);
	void Ant_TransformComponent_GetScale(uint64_t entityID, glm::vec3* outScale);
	void Ant_TransformComponent_SetScale(uint64_t entityID, glm::vec3* inScale);

	void* Ant_MeshComponent_GetMesh(uint64_t entityID);
	void Ant_MeshComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh);

	void Ant_RigidBody2DComponent_ApplyLinearImpulse(uint64_t entityID, glm::vec2* impulse, glm::vec2* offset, bool wake);
	void Ant_RigidBody2DComponent_GetLinearVelocity(uint64_t entityID, glm::vec2* outVelocity);
	void Ant_RigidBody2DComponent_SetLinearVelocity(uint64_t entityID, glm::vec2* velocity);

	RigidBodyComponent::Type Ant_RigidBodyComponent_GetBodyType(uint64_t entityID);
	void Ant_RigidBodyComponent_AddForce(uint64_t entityID, glm::vec3* force, ForceMode foceMode);
	void Ant_RigidBodyComponent_AddTorque(uint64_t entityID, glm::vec3* torque, ForceMode forceMode);
	void Ant_RigidBodyComponent_GetLinearVelocity(uint64_t entityID, glm::vec3* outVelocity);
	void Ant_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, glm::vec3* velocity);
	void Ant_RigidBodyComponent_GetAngularVelocity(uint64_t entityID, glm::vec3* outVelocity);
	void Ant_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, glm::vec3* velocity);
	void Ant_RigidBodyComponent_Rotate(uint64_t entityID, glm::vec3* rotation);
	uint32_t Ant_RigidBodyComponent_GetLayer(uint64_t entityID);
	float Ant_RigidBodyComponent_GetMass(uint64_t entityID);
	void Ant_RigidBodyComponent_SetMass(uint64_t entityID, float mass);

	// Renderer
	// Texture2D
	void* Ant_Texture2D_Constructor(uint32_t width, uint32_t height);
	void Ant_Texture2D_Destructor(Ref<Texture2D>* _this);
	void Ant_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* inData, int32_t count);

	// Material
	void Ant_Material_Destructor(Ref<Material>* _this);
	void Ant_Material_SetFloat(Ref<Material>* _this, MonoString* uniform, float value);
	void Ant_Material_SetTexture(Ref<Material>* _this, MonoString* uniform, Ref<Texture2D>* texture);

	void Ant_MaterialInstance_Destructor(Ref<Material>* _this);
	void Ant_MaterialInstance_SetFloat(Ref<Material>* _this, MonoString* uniform, float value);
	void Ant_MaterialInstance_SetVector3(Ref<Material>* _this, MonoString* uniform, glm::vec3* value);
	void Ant_MaterialInstance_SetVector4(Ref<Material>* _this, MonoString* uniform, glm::vec4* value);
	void Ant_MaterialInstance_SetTexture(Ref<Material>* _this, MonoString* uniform, Ref<Texture2D>* texture);

	// Mesh
	Ref<Mesh>* Ant_Mesh_Constructor(MonoString* filepath);
	void Ant_Mesh_Destructor(Ref<Mesh>* _this);
	Ref<Material>* Ant_Mesh_GetMaterial(Ref<Mesh>* inMesh);
	Ref<Material>* Ant_Mesh_GetMaterialByIndex(Ref<Mesh>* inMesh, int index);
	int Ant_Mesh_GetMaterialCount(Ref<Mesh>* inMesh);

	void* Ant_MeshFactory_CreatePlane(float width, float height);
}}