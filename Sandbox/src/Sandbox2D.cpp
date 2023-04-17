#include "Sandbox2D.h"

#include <imgui.h>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/type_ptr.hpp>

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1600.0f / 900.0f)
{

}

void Sandbox2D::OnAttach()
{

}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(Ant::Timestep ts)
{
	// Updata
	m_CameraController.OnUpdata(ts);

	// Render
	Ant::RendererCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	Ant::RendererCommand::Clear();

	Ant::Renderer2D::BeginScene(m_CameraController.GetCamera());

	Ant::Renderer2D::DrawQuad({ 0.0f,0.0f }, { 1.0f,1.0f }, { 0.8f, 0.3f, 0.2f, 1.0f });

	Ant::Renderer2D::EndScene();


	/*std::dynamic_pointer_cast<Ant::OpenGLShader>(m_Shader)->Bind();
	std::dynamic_pointer_cast<Ant::OpenGLShader>(m_Shader)->UploadUniformFloat4("u_Color", m_Color);*/
}

void Sandbox2D::OnImGuiRender()
{
	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_Color));
	ImGui::End();
}

void Sandbox2D::OnEvent(Ant::Event& event)
{
	m_CameraController.OnEvent(event);
}
