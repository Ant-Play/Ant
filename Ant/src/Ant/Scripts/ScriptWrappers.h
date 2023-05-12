#pragma once
#include "Ant/Scripts/ScriptEngine.h"
#include "Ant/Core/KeyCodes.h"


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

	// Entity
	void Ant_Entity_GetTransform(uint64_t entityID, glm::mat4* outTransform);
	void Ant_Entity_SetTransform(uint64_t entityID, glm::mat4* inTransform);
	void Ant_Entity_CreateComponent(uint64_t entityID, void* type);
	bool Ant_Entity_HasComponent(uint64_t entityID, void* type);

	void* Ant_MeshComponent_GetMesh(uint64_t entityID);
	void Ant_MeshComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh);

	// Renderer
	// Texture2D
	void* Ant_Texture2D_Constructor(uint32_t width, uint32_t height);
	void Ant_Texture2D_Destructor(Ref<Texture2D>* _this);
	void Ant_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* inData, int32_t count);

	// Material
	void Ant_Material_Destructor(Ref<Material>* _this);
	void Ant_Material_SetFloat(Ref<Material>* _this, MonoString* uniform, float value);
	void Ant_Material_SetTexture(Ref<Material>* _this, MonoString* uniform, Ref<Texture2D>* texture);

	void Ant_MaterialInstance_Destructor(Ref<MaterialInstance>* _this);
	void Ant_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, MonoString* uniform, float value);
	void Ant_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec3* value);
	void Ant_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, MonoString* uniform, Ref<Texture2D>* texture);

	// Mesh
	Ref<Mesh>* Ant_Mesh_Constructor(MonoString* filepath);
	void Ant_Mesh_Destructor(Ref<Mesh>* _this);
	Ref<Material>* Ant_Mesh_GetMaterial(Ref<Mesh>* inMesh);
	Ref<MaterialInstance>* Ant_Mesh_GetMaterialByIndex(Ref<Mesh>* inMesh, int index);
	int Ant_Mesh_GetMaterialCount(Ref<Mesh>* inMesh);

	void* Ant_MeshFactory_CreatePlane(float width, float height);
}}