#include "antpch.h"
#include "Ant/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Ant {

	Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;


	void Renderer::Init()
	{
		RendererCommand::Init();
	}

	// TODO
	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	// TODO
	void Renderer::EndScene()
	{

	}


	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);

		vertexArray->Bind();
		RendererCommand::DrawIndexed(vertexArray);
	}

}
