
#include "Sandbox2D.h"

#include <imgui.h>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/type_ptr.hpp>
#include "Ant/Debug/Instrumentor.h"

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1600.0f / 900.0f)
{
	
}

void Sandbox2D::OnAttach()
{
	ANT_PROFILE_FUNCTION();

	m_Texture = Ant::Texture2D::Create("assets/textures/Checkerboard.png");
	m_ShipTexture = Ant::Texture2D::Create("assets/textures/Ship.png");
}

void Sandbox2D::OnDetach()
{
	ANT_PROFILE_FUNCTION();
}

void Sandbox2D::OnUpdate(Ant::Timestep ts)
{
	ANT_PROFILE_FUNCTION();
	// Updata
	m_CameraController.OnUpdata(ts);

	// Render
	{
		ANT_PROFILE_SCOPE("Renderer Prep");
		Ant::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Ant::RenderCommand::Clear();

	}

	{
		ANT_PROFILE_SCOPE("Renderer Draw");

		Ant::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Ant::Renderer2D::DrawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.3f, 0.2f, 1.0f });
		Ant::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		Ant::Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_Texture);
		Ant::Renderer2D::DrawQuad({ m_Position.x, m_Position.y, 0.5f }, { 1.0f, 1.3f }, /*glm::radians(GetRotation()),*/ m_ShipTexture);
	}
		Ant::Renderer2D::EndScene();
}

void Sandbox2D::OnImGuiRender()
{
	ANT_PROFILE_FUNCTION();

	ImGui::Begin("Settings");
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_Color));
	ImGui::End();
}

void Sandbox2D::OnEvent(Ant::Event& event)
{
	m_CameraController.OnEvent(event);
}
