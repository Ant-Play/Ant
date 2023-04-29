
#include "EditorLayer.h"

#include <imgui/imgui.h>
#include "Platform/OpenGL/OpenGLShader.h"
#include <glm/gtc/type_ptr.hpp>
#include "Ant/Debug/Instrumentor.h"

namespace Ant {
	EditorLayer::EditorLayer()
		: Layer("Sandbox2D"), m_CameraController(1600.0f / 900.0f)
	{

	}

	void EditorLayer::OnAttach()
	{
		ANT_PROFILE_FUNCTION();

		m_Texture = Ant::Texture2D::Create("assets/textures/Checkerboard.png");

		Ant::FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 780;
		m_Framebuffer = Ant::Framebuffer::Create(fbSpec);
	}

	void EditorLayer::OnDetach()
	{
		ANT_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Ant::Timestep ts)
	{
		ANT_PROFILE_FUNCTION();
		// Updata
		m_CameraController.OnUpdata(ts);

		// Render
		// Reset stats here
		Ant::Renderer2D::ResetStats();
		{
			ANT_PROFILE_SCOPE("Renderer Prep");
			m_Framebuffer->Bind();
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
			m_Framebuffer->Unbind();
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		ANT_PROFILE_FUNCTION();

		static bool dockingEnabled = true;

		if (dockingEnabled)
		{
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
			ImGui::End();


			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
			ImGui::Begin("Viewport");
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize))
			{
				m_Framebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
				m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

				m_CameraController.ResizeBounds(viewportPanelSize.x, viewportPanelSize.y);
			}
			ANT_WARN("Vireport Size: {0}, {1}", viewportPanelSize.x, viewportPanelSize.y);
			uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::End();
			ImGui::PopStyleVar();

			ImGui::End();
		}
		else
		{
			ImGui::Begin("Settings");

			auto stats = Ant::Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats: ");
			ImGui::Text("Draw Calls: %d ", stats.DrawCalls);
			ImGui::Text("Quads: %d ", stats.QuadCount);
			ImGui::Text("Vertices: %d ", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d ", stats.GetTotalIndexCount());
			ImGui::ColorEdit4("Square Color", glm::value_ptr(m_Color));
			
			uint32_t textureID = m_Texture->GetRendererID();
			ImGui::Image((void*)textureID, ImVec2(1280, 720));
			ImGui::End();
		}
	}

	void EditorLayer::OnEvent(Ant::Event& event)
	{
		m_CameraController.OnEvent(event);
	}
}
