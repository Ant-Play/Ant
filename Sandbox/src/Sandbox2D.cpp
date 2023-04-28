
#include "Sandbox2D.h"

#include <imgui.h>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/type_ptr.hpp>
#include "Ant/Debug/Instrumentor.h"

static const uint32_t s_MapWidth = 24;
static const char* s_MapTiles =
"WWWWWWWWWWWWWWWWWWWWWWWW"
"WWWWWWWDDDDDDDWWWWWWWWWW"
"WWWWWDDDDDDDDDDDWWWWWWWW"
"WWWWDDDDDDDDDDDDDDDWWWWW"
"WWWDDDDDDDDDDDDDDDDDDWWW"
"WWDDDWWWDDDDDDDDDDDDDDWW"
"WDDDDWWWDDDDDDDDDDDDDDWW"
"WWDDDDDDDDDDDDDDDDDDDWWW"
"WWWDDDDDDDDDDDDDDDDDDWWW"
"WWWWDDDDDDDDDDDDDDDWWWWW"
"WWWWWDDDDDDDDDDDWWWWWWWW"
"WWWWWWWDDDDDDDWWWWWWWWWW"
"WWWWWWWWWDDDWWWWWWWWWWWW"
"WWWWWWWWWWWWWWWWWWWWWWWW";

Sandbox2D::Sandbox2D()
	: Layer("Sandbox2D"), m_CameraController(1600.0f / 900.0f)
{
	
}

void Sandbox2D::OnAttach()
{
	ANT_PROFILE_FUNCTION();

	m_Texture = Ant::Texture2D::Create("assets/textures/Checkerboard.png");
	m_SpriteSheet = Ant::Texture2D::Create("assets/game/textures/RPGpack_sheet_2X.png");

	m_TextureStairs = Ant::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0, 11 }, { 128, 128 });
	m_TextureTree = Ant::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 2, 1 }, { 128, 128 }, { 1, 2 });

	m_MapWidth = s_MapWidth;
	m_MapHeight = strlen(s_MapTiles) / s_MapWidth;
	ANT_INFO("{0}, {1}", m_MapWidth, m_MapHeight);

	s_TextureMap['D'] = Ant::SubTexture2D::CreateFromCoords(m_SpriteSheet, {6, 11}, {128, 128});
	s_TextureMap['W'] = Ant::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11, 11}, {128, 128});


	m_Particle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
	m_Particle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
	m_Particle.SizeBegin = 0.5f, m_Particle.SizeVariation = 0.3f, m_Particle.SizeEnd = 0.0f;
	m_Particle.LifeTime = 5.0f;
	m_Particle.Velocity = { 0.0f, 0.0f };
	m_Particle.VelocityVariation = { 3.0f, 1.0f };
	m_Particle.Position = { 0.0f, 0.0f };

	m_CameraController.SetZoomLevel(5.0f);
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

#if 1
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
#endif

	if (Ant::Input::IsMouseButtonPressed(Ant::Mouse::ButtonLeft))
	{
		auto [x, y] = Ant::Input::GetMousePosition();
		auto width = Ant::Application::Get().GetWindow().GetWidth();
		auto height = Ant::Application::Get().GetWindow().GetHeight();

		auto bounds = m_CameraController.GetBounds();
		auto pos = m_CameraController.GetCamera().GetPosition();
		x = (x / width) * bounds.GetWidth() - bounds.GetHeight() * 0.9f;
		y = bounds.GetHeight() * 0.5f - (y / height) * bounds.GetHeight();
		m_Particle.Position = { x + pos.x, y + pos.y };
		for (int i = 0; i < 20; i++)
			m_ParticleSystem.Emit(m_Particle);
	}

#if 0
	Ant::Renderer2D::BeginScene(m_CameraController.GetCamera());

	for (uint32_t y = 0; y < m_MapHeight; y++)
	{
		for (uint32_t x = 0; x < m_MapWidth; x++)
		{
			char tileType = s_MapTiles[x + y * m_MapWidth];
			Ant::Ref<Ant::SubTexture2D> texture;
			if (s_TextureMap.find(tileType) != s_TextureMap.end())
				texture = s_TextureMap[tileType];
			else
				texture = m_TextureTree;
			Ant::Renderer2D::DrawQuad({ x - m_MapWidth / 2.0f, y - m_MapHeight / 2.0f, 0.5f }, { 1.0f, 1.0f }, texture);
		}
	}
	Ant::Renderer2D::EndScene();
#endif
	m_ParticleSystem.OnUpdate(ts);
	m_ParticleSystem.OnRender(m_CameraController.GetCamera());
}

void Sandbox2D::OnImGuiRender()
{
	ANT_PROFILE_FUNCTION();


	static bool dockspaceOpen = true;
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.

			if (ImGui::MenuItem("Exit")) Ant::Application::Get().Close();
			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}


	ImGui::Begin("Settings");

	auto stats = Ant::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats: ");
	ImGui::Text("Draw Calls: %d ", stats.DrawCalls);
	ImGui::Text("Quads: %d ", stats.QuadCount);
	ImGui::Text("Vertices: %d ", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d ", stats.GetTotalIndexCount());
	ImGui::ColorEdit4("Square Color", glm::value_ptr(m_Color));

	uint32_t textureID = m_Texture->GetRendererID();
	ImGui::Image((void*)textureID, ImVec2(64.0f, 64.0f));
	ImGui::End();

	ImGui::End();
}

void Sandbox2D::OnEvent(Ant::Event& event)
{
	m_CameraController.OnEvent(event);
}
