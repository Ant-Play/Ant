#include "antpch.h"
#include "ScriptEngineRegistry.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Ant/Scene/Entity.h"
#include "ScriptWrappers.h"

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
		Component_RegisterType(RigidBodyComponent);
		Component_RegisterType(BoxColliderComponent);
		Component_RegisterType(SphereColliderComponent);
	}

	void ScriptEngineRegistry::RegisterAll()
	{
		InitComponentTypes();

		mono_add_internal_call("Ant.Noise::PerlinNoise_Native", Ant::Script::Ant_Noise_PerlinNoise);

		mono_add_internal_call("Ant.Physics::Raycast_Native", Ant::Script::Ant_Physics_Raycast);
		mono_add_internal_call("Ant.Physics::OverlapBox_Native", Ant::Script::Ant_Physics_OverlapBox);
		mono_add_internal_call("Ant.Physics::OverlapCapsule_Native", Ant::Script::Ant_Physics_OverlapCapsule);
		mono_add_internal_call("Ant.Physics::OverlapSphere_Native", Ant::Script::Ant_Physics_OverlapSphere);
		mono_add_internal_call("Ant.Physics::OverlapBoxNonAlloc_Native", Ant::Script::Ant_Physics_OverlapBoxNonAlloc);
		mono_add_internal_call("Ant.Physics::OverlapCapsuleNonAlloc_Native", Ant::Script::Ant_Physics_OverlapCapsuleNonAlloc);
		mono_add_internal_call("Ant.Physics::OverlapSphereNonAlloc_Native", Ant::Script::Ant_Physics_OverlapSphereNonAlloc);

		mono_add_internal_call("Ant.Entity::CreateComponent_Native", Ant::Script::Ant_Entity_CreateComponent);
		mono_add_internal_call("Ant.Entity::HasComponent_Native", Ant::Script::Ant_Entity_HasComponent);
		mono_add_internal_call("Ant.Entity::FindEntityByTag_Native", Ant::Script::Ant_Entity_FindEntityByTag);

		mono_add_internal_call("Ant.TransformComponent::GetTransform_Native", Ant::Script::Ant_TransformComponent_GetTransform);
		mono_add_internal_call("Ant.TransformComponent::SetTransform_Native", Ant::Script::Ant_TransformComponent_SetTransform);
		mono_add_internal_call("Ant.TransformComponent::GetTranslation_Native", Ant::Script::Ant_TransformComponent_GetTranslation);
		mono_add_internal_call("Ant.TransformComponent::SetTranslation_Native", Ant::Script::Ant_TransformComponent_SetTranslation);
		mono_add_internal_call("Ant.TransformComponent::GetRotation_Native", Ant::Script::Ant_TransformComponent_GetRotation);
		mono_add_internal_call("Ant.TransformComponent::SetRotation_Native", Ant::Script::Ant_TransformComponent_SetRotation);
		mono_add_internal_call("Ant.TransformComponent::GetScale_Native", Ant::Script::Ant_TransformComponent_GetScale);
		mono_add_internal_call("Ant.TransformComponent::SetScale_Native", Ant::Script::Ant_TransformComponent_SetScale);

		mono_add_internal_call("Ant.MeshComponent::GetMesh_Native", Ant::Script::Ant_MeshComponent_GetMesh);
		mono_add_internal_call("Ant.MeshComponent::SetMesh_Native", Ant::Script::Ant_MeshComponent_SetMesh);

		mono_add_internal_call("Ant.RigidBody2DComponent::ApplyLinearImpulse_Native", Ant::Script::Ant_RigidBody2DComponent_ApplyLinearImpulse);
		mono_add_internal_call("Ant.RigidBody2DComponent::GetLinearVelocity_Native", Ant::Script::Ant_RigidBody2DComponent_GetLinearVelocity);
		mono_add_internal_call("Ant.RigidBody2DComponent::SetLinearVelocity_Native", Ant::Script::Ant_RigidBody2DComponent_SetLinearVelocity);

		mono_add_internal_call("Ant.RigidBodyComponent::GetBodyType_Native", Ant::Script::Ant_RigidBodyComponent_GetBodyType);
		mono_add_internal_call("Ant.RigidBodyComponent::AddForce_Native", Ant::Script::Ant_RigidBodyComponent_AddForce);
		mono_add_internal_call("Ant.RigidBodyComponent::AddTorque_Native", Ant::Script::Ant_RigidBodyComponent_AddTorque);
		mono_add_internal_call("Ant.RigidBodyComponent::GetLinearVelocity_Native", Ant::Script::Ant_RigidBodyComponent_GetLinearVelocity);
		mono_add_internal_call("Ant.RigidBodyComponent::SetLinearVelocity_Native", Ant::Script::Ant_RigidBodyComponent_SetLinearVelocity);
		mono_add_internal_call("Ant.RigidBodyComponent::GetAngularVelocity_Native", Ant::Script::Ant_RigidBodyComponent_GetAngularVelocity);
		mono_add_internal_call("Ant.RigidBodyComponent::SetAngularVelocity_Native", Ant::Script::Ant_RigidBodyComponent_SetAngularVelocity);
		mono_add_internal_call("Ant.RigidBodyComponent::Rotate_Native", Ant::Script::Ant_RigidBodyComponent_Rotate);
		mono_add_internal_call("Ant.RigidBodyComponent::GetLayer_Native", Ant::Script::Ant_RigidBodyComponent_GetLayer);
		mono_add_internal_call("Ant.RigidBodyComponent::GetMass_Native", Ant::Script::Ant_RigidBodyComponent_GetMass);
		mono_add_internal_call("Ant.RigidBodyComponent::SetMass_Native", Ant::Script::Ant_RigidBodyComponent_SetMass);

		mono_add_internal_call("Ant.Input::IsKeyPressed_Native", Ant::Script::Ant_Input_IsKeyPressed);
		mono_add_internal_call("Ant.Input::IsMouseButtonPressed_Native", Ant::Script::Ant_Input_IsMouseButtonPressed);
		mono_add_internal_call("Ant.Input::GetMousePosition_Native", Ant::Script::Ant_Input_GetMousePosition);
		mono_add_internal_call("Ant.Input::SetCursorMode_Native", Ant::Script::Ant_Input_SetCursorMode);
		mono_add_internal_call("Ant.Input::GetCursorMode_Native", Ant::Script::Ant_Input_GetCursorMode);

		mono_add_internal_call("Ant.Texture2D::Constructor_Native", Ant::Script::Ant_Texture2D_Constructor);
		mono_add_internal_call("Ant.Texture2D::Destructor_Native", Ant::Script::Ant_Texture2D_Destructor);
		mono_add_internal_call("Ant.Texture2D::SetData_Native", Ant::Script::Ant_Texture2D_SetData);

		mono_add_internal_call("Ant.Material::Destructor_Native", Ant::Script::Ant_Material_Destructor);
		mono_add_internal_call("Ant.Material::SetFloat_Native", Ant::Script::Ant_Material_SetFloat);
		mono_add_internal_call("Ant.Material::SetTexture_Native", Ant::Script::Ant_Material_SetTexture);

		mono_add_internal_call("Ant.MaterialInstance::Destructor_Native", Ant::Script::Ant_MaterialInstance_Destructor);
		mono_add_internal_call("Ant.MaterialInstance::SetFloat_Native", Ant::Script::Ant_MaterialInstance_SetFloat);
		mono_add_internal_call("Ant.MaterialInstance::SetVector3_Native", Ant::Script::Ant_MaterialInstance_SetVector3);
		mono_add_internal_call("Ant.MaterialInstance::SetVector4_Native", Ant::Script::Ant_MaterialInstance_SetVector4);
		mono_add_internal_call("Ant.MaterialInstance::SetTexture_Native", Ant::Script::Ant_MaterialInstance_SetTexture);

		mono_add_internal_call("Ant.Mesh::Constructor_Native", Ant::Script::Ant_Mesh_Constructor);
		mono_add_internal_call("Ant.Mesh::Destructor_Native", Ant::Script::Ant_Mesh_Destructor);
		mono_add_internal_call("Ant.Mesh::GetMaterial_Native", Ant::Script::Ant_Mesh_GetMaterial);
		mono_add_internal_call("Ant.Mesh::GetMaterialByIndex_Native", Ant::Script::Ant_Mesh_GetMaterialByIndex);
		mono_add_internal_call("Ant.Mesh::GetMaterialCount_Native", Ant::Script::Ant_Mesh_GetMaterialCount);

		mono_add_internal_call("Ant.MeshFactory::CreatePlane_Native", Ant::Script::Ant_MeshFactory_CreatePlane);
	}
}