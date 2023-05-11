#include "antpch.h"
#include "Ant/Scripts/ScriptWrappers.h"


#include "Ant/Core/Inputs.h"
#include "Ant/Core/Math/Noise.h"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

namespace Ant{
	extern std::unordered_map<uint32_t, Scene*> s_ActiveScenes;
	extern std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
	extern std::unordered_map<MonoType*, std::function<void(Entity&)>> s_CreateComponentFuncs;
}


namespace Ant { namespace Script {

		enum class ComponentID
		{
			None = 0,
			Transform = 1,
			Mesh = 2,
			Script = 3,
			SpriteRenderer = 4,
		};



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

		////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////
		// Entity //////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////

		void Ant_Entity_GetTransform(uint32_t sceneID, uint32_t entityID, glm::mat4* transform)
		{
			ANT_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			auto& transformComponent = entity.GetComponent<TransformComponent>();
			memcpy(glm::value_ptr(transformComponent.Transform), transform, sizeof(glm::mat4));
		}

		void Ant_Entity_SetTransform(uint32_t sceneID, uint32_t entityID, glm::mat4* transform)
		{
			ANT_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			auto& transformComponent = entity.GetComponent<TransformComponent>();
			memcpy(transform, glm::value_ptr(transformComponent.Transform), sizeof(glm::mat4));
		}

		void Ant_Entity_CreateComponent(uint32_t sceneID, uint32_t entityID, void* type)
		{
			ANT_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");
			MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			s_CreateComponentFuncs[monoType](entity);
		}

		bool Ant_Entity_HasComponent(uint32_t sceneID, uint32_t entityID, void* type)
		{
			ANT_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");
			MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			return s_HasComponentFuncs[monoType](entity);
		}

		void* Ant_MeshComponent_GetMesh(uint32_t sceneID, uint32_t entityID)
		{
			ANT_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			auto& meshComponent = entity.GetComponent<MeshComponent>();
			return new Ref<Mesh>(meshComponent.Mesh);
		}

		void Ant_MeshComponent_SetMesh(uint32_t sceneID, uint32_t entityID, Ref<Mesh>* mesh)
		{
			ANT_CORE_ASSERT(s_ActiveScenes.find(sceneID) != s_ActiveScenes.end(), "Invalid Scene ID!");

			Scene* scene = s_ActiveScenes[sceneID];
			Entity entity((entt::entity)entityID, scene);
			auto& meshComponent = entity.GetComponent<MeshComponent>();
			meshComponent.Mesh = mesh ? *mesh : nullptr;
		}

		Ref<Mesh>* Ant_Mesh_Constructor(MonoString* filepath)
		{
			std::string path = mono_string_to_utf8(filepath);
			return new Ref<Mesh>(new Mesh(path));
		}

		void Ant_Mesh_Destructor(Ref<Mesh>* _this)
		{
			Ref<Mesh>* instance = (Ref<Mesh>*)_this;
			delete _this;
		}

		Ref<Material>* Ant_Mesh_GetMaterial(Ref<Mesh>* _this)
		{
			Ref<Mesh>& mesh = *(Ref<Mesh>*)_this;
			return new Ref<Material>(mesh->GetMaterial());
		}

		Ref<MaterialInstance>* Ant_Mesh_GetMaterialByIndex(Ref<Mesh>* _this, uint32_t index)
		{
			Ref<Mesh>& mesh = *static_cast<Ref<Mesh>*>(_this);
			const auto& materials = mesh->GetMaterials();

			ANT_CORE_ASSERT(index < materials.size());
			return new Ref<MaterialInstance>(materials[index]);
		}

		int Ant_Mesh_GetMaterialCount(Ref<Mesh>* _this)
		{
			Ref<Mesh>& mesh = *static_cast<Ref<Mesh>*>(_this);
			const auto& materials = mesh->GetMaterials();
			return materials.size();
		}

		void* Ant_Texture2D_Constructor(uint32_t width, uint32_t height)
		{
			auto result = Texture2D::Create(TextureFormat::RGBA, width, height);
			return new Ref<Texture2D>(result);
		}

		void Ant_Texture2D_Destructor(Ref<Texture2D>* _this)
		{
			delete _this;
		}

		void Ant_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* data, int32_t size)
		{
			Ref<Texture2D>& instance = *_this;

			uint32_t dataSize = size * sizeof(glm::vec4) / 4;

			instance->Lock();
			Buffer buffer = instance->GetWriteableBuffer();
			ANT_CORE_ASSERT(dataSize <= buffer.Size);
			// Convert RGBA32F color to RGBA8
			uint8_t* pixels = (uint8_t*)buffer.Data;
			uint32_t index = 0;
			for (int i = 0; i < instance->GetWidth() * instance->GetHeight(); i++)
			{
				glm::vec4& value = mono_array_get(data, glm::vec4, i);
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

		void Ant_MaterialInstance_Destructor(Ref<MaterialInstance>* _this)
		{
			delete _this;
		}

		void Ant_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, MonoString* uniform, float value)
		{
			Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
			instance->Set(mono_string_to_utf8(uniform), value);
		}

		void Ant_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec3* value)
		{
			Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
			instance->Set(mono_string_to_utf8(uniform), *value);
		}

		void Ant_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, MonoString* uniform, Ref<Texture2D>* texture)
		{
			Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
			instance->Set(mono_string_to_utf8(uniform), *texture);
		}

		void* Ant_MeshFactory_CreatePlane(float width, float height)
		{
			// TODO: Implement properly with MeshFactory class!
			return new Ref<Mesh>(new Mesh("assets/models/Plane1m.obj"));
		}
}}