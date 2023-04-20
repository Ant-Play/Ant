#include "antpch.h"
#include "Ant/Renderer/Renderer.h"
#include "Ant/Renderer/Renderer2D.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Ant {

	Scope<Renderer::SceneData> Renderer::m_SceneData = CreateScope<Renderer::SceneData>();


	void Renderer::Init()
	{
		ANT_PROFILE_FUNCTION();

		RendererCommand::Init();
		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RendererCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	// TODO
	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const Ref<Shader>&shader, const Ref<VertexArray>&vertexArray, const glm::mat4 & transform)
	{
		shader->Bind();
		shader->SetMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
		shader->SetMat4("u_Transform", transform);

		vertexArray->Bind();
		RendererCommand::DrawIndexed(vertexArray);
	}

}
