#pragma once
#include "Ant/Renderer/Texture.h"
#include "Ant/Renderer/Mesh.h"
#include "Ant/Renderer/Camera.h"
#include "Ant/Core/Ref.h"

#include <glm/glm.hpp>


namespace Ant{

	struct TagComponent
	{
		std::string Tag;

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct TransformComponent
	{
		glm::mat4 Transform;

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }
	};

	struct MeshComponent
	{
		Ref<Ant::Mesh> Mesh;

		operator Ref<Ant::Mesh>() { return Mesh; }
	};

	struct ScriptComponent
	{
		// TODO: C# script
		std::string ModuleName;
	};

	struct CameraComponent
	{
		Ant::Camera Camera;
		bool Primary = true;

		operator Ant::Camera() { return Camera; }
		operator const Ant::Camera() const { return Camera; }
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		float TillingFactor = 1.0f;
	};
}
