#include "antpch.h"
#include "Ant/Scripts/ScriptEngineRegistry.h"

#include "Ant/Scripts/ScriptWrappers.h"
#include "Ant/Scene/Entity.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <iostream>

namespace Ant {

	std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
	std::unordered_map<MonoType*, std::function<void(Entity&)>> s_CreateComponentFuncs;

	extern MonoImage* s_CoreAssemblyImage;

#define Component_RegisterType(Type) \
	{\
		MonoType* type = mono_reflection_type_from_name("Ant." #Type, s_CoreAssemblyImage);\
		if (type) {\
			uint32_t id = mono_type_get_type(type);\
			s_HasComponentFuncs[type] = [](Entity& entity) { return entity.HasComponent<Type>(); };\
			s_CreateComponentFuncs[type] = [](Entity& entity) { entity.AddComponent<Type>(); };\
		} else {\
			ANT_CORE_ERROR("No C# component class found for " #Type "!");\
		}\
	}

	static void InitComponentTypes()
	{
		Component_RegisterType(TagComponent);
		Component_RegisterType(TransformComponent);
		Component_RegisterType(MeshComponent);
		Component_RegisterType(ScriptComponent);
		Component_RegisterType(CameraComponent);
		Component_RegisterType(SpriteRendererComponent);
		Component_RegisterType(RigidBody2DComponent);
		Component_RegisterType(BoxCollider2DComponent);
	}

	void ScriptEngineRegistry::RegisterAll()
	{
		InitComponentTypes();

		mono_add_internal_call("Ant.Noise::PerlinNoise_Native", Ant::Script::Ant_Noise_PerlinNoise);

		mono_add_internal_call("Ant.Entity::GetTransform_Native", Ant::Script::Ant_Entity_GetTransform);
		mono_add_internal_call("Ant.Entity::SetTransform_Native", Ant::Script::Ant_Entity_SetTransform);
		mono_add_internal_call("Ant.Entity::CreateComponent_Native", Ant::Script::Ant_Entity_CreateComponent);
		mono_add_internal_call("Ant.Entity::HasComponent_Native", Ant::Script::Ant_Entity_HasComponent);

		mono_add_internal_call("Ant.MeshComponent::GetMesh_Native", Ant::Script::Ant_MeshComponent_GetMesh);
		mono_add_internal_call("Ant.MeshComponent::SetMesh_Native", Ant::Script::Ant_MeshComponent_SetMesh);

		mono_add_internal_call("Ant.RigidBody2DComponent::ApplyLinearImpulse_Native", Ant::Script::Ant_RigidBody2DComponent_ApplyLinearImpulse);


		mono_add_internal_call("Ant.Input::IsKeyPressed_Native", Ant::Script::Ant_Input_IsKeyPressed);

		mono_add_internal_call("Ant.Texture2D::Constructor_Native", Ant::Script::Ant_Texture2D_Constructor);
		mono_add_internal_call("Ant.Texture2D::Destructor_Native", Ant::Script::Ant_Texture2D_Destructor);
		mono_add_internal_call("Ant.Texture2D::SetData_Native", Ant::Script::Ant_Texture2D_SetData);

		mono_add_internal_call("Ant.Material::Destructor_Native", Ant::Script::Ant_Material_Destructor);
		mono_add_internal_call("Ant.Material::SetFloat_Native", Ant::Script::Ant_Material_SetFloat);
		mono_add_internal_call("Ant.Material::SetTexture_Native", Ant::Script::Ant_Material_SetTexture);

		mono_add_internal_call("Ant.MaterialInstance::Destructor_Native", Ant::Script::Ant_MaterialInstance_Destructor);
		mono_add_internal_call("Ant.MaterialInstance::SetFloat_Native", Ant::Script::Ant_MaterialInstance_SetFloat);
		mono_add_internal_call("Ant.MaterialInstance::SetVector3_Native", Ant::Script::Ant_MaterialInstance_SetVector3);
		mono_add_internal_call("Ant.MaterialInstance::SetTexture_Native", Ant::Script::Ant_MaterialInstance_SetTexture);

		mono_add_internal_call("Ant.Mesh::Constructor_Native", Ant::Script::Ant_Mesh_Constructor);
		mono_add_internal_call("Ant.Mesh::Destructor_Native", Ant::Script::Ant_Mesh_Destructor);
		mono_add_internal_call("Ant.Mesh::GetMaterial_Native", Ant::Script::Ant_Mesh_GetMaterial);
		mono_add_internal_call("Ant.Mesh::GetMaterialByIndex_Native", Ant::Script::Ant_Mesh_GetMaterialByIndex);
		mono_add_internal_call("Ant.Mesh::GetMaterialCount_Native", Ant::Script::Ant_Mesh_GetMaterialCount);

		mono_add_internal_call("Ant.MeshFactory::CreatePlane_Native", Ant::Script::Ant_MeshFactory_CreatePlane);

		// static bool IsKeyPressed(KeyCode key) { return s_Instance->IsKeyPressedImpl(key); }
		// 
		// static bool IsMouseButtonPressed(MouseCode button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		// static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		// static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		// static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
	}
}