
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
	// Reset stats here
	Ant::Renderer2D::ResetStats();
	{
		ANT_PROFILE_SCOPE("Renderer Prep");
		Ant::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Ant::RenderCommand::Clear();

	}

	{
		static float rotation = 0.0f;
		rotation += ts * 50.0f;

		ANT_PROFILE_SCOPE("Renderer Draw");

		Ant::Renderer2D::BeginScene(m_CameraController.GetCamera());
		Ant::Renderer2D::DrawRotatedQuad({ 1.0f,0.0f }, { 0.8f,0.8f }, -45.0f, { 0.8f,0.2f,0.3f,1.0f });
		Ant::Renderer2D::DrawQuad({ -1.0f,0.0f }, { 0.8f,0.8f }, { 0.8f,0.2f,0.3f,1.0f });
		Ant::Renderer2D::DrawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
		Ant::Renderer2D::DrawQuad(glm::vec3(0.0f), { 20.0f, 20.0f }, m_Texture, 10.0f);
		Ant::Renderer2D::DrawRotatedQuad({ -2.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, rotation, m_Texture, 20.0f);
		Ant::Renderer2D::EndScene();

		Ant::Renderer2D::BeginScene(m_CameraController.GetCamera());
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f,  (y + 5.0f) / 10.0f, 0.7f };
				Ant::Renderer2D::DrawQuad({ x, y, 0.2f }, { 0.45f, 0.45f }, color);
			}
		}
		Ant::Renderer2D::EndScene();
	}
}

void Sandbox2D::OnImGuiRender()
{
	ANT_PROFILE_FUNCTION();

	ImGui::Begin("Settings");

	auto stats = Ant::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats: ");
	ImGui::Text("Draw Calls: %d ",stats.DrawCalls);
	ImGui::Text("Quads: %d ",stats.QuadCount);
	ImGui::Text("Vertices: %d ",stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d ",stats.GetTotalIndexCount());
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_Color));
	ImGui::End();
}

void Sandbox2D::OnEvent(Ant::Event& event)
{
	m_CameraController.OnEvent(event);
}
