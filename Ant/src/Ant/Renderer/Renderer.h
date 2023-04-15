#pragma once
#include "Ant/Core.h"
#include "Ant/Renderer/RendererCommand.h"
#include "Ant/Renderer/VertexArray.h"
#include "Ant/Renderer/OrthographicCamera.h"
#include "Ant/Renderer/Shader.h"

namespace Ant{

	class Renderer
	{
	public:
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* m_SceneData;
	};
}
