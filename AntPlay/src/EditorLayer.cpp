
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

		m_Texture = Texture2D::Create("assets/textures/Checkerboard.png");

		FramebufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 780;
		m_Framebuffer = Framebuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();

		auto square = m_ActiveScene->CreateEntity("Square");
		square.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });

		m_SquareEntity = square;

		m_MainCamera = m_ActiveScene->CreateEntity("Camera Entity");
		m_MainCamera.AddComponent<CameraComponent>();

		m_SecondCamera = m_ActiveScene->CreateEntity("Clip-Camera Entity");
		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
		cc.Primary = false;

	}

	void EditorLayer::OnDetach()
	{
		ANT_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		ANT_PROFILE_FUNCTION();

		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&  // zero sized framebuffer is invaild
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.ResizeBounds(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// Updata
		if(m_ViewportFocused)
			m_CameraController.OnUpdata(ts);

		// Render
		// Reset stats here
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		RenderCommand::Clear();

		// Update scene
		m_ActiveScene->OnUpdate(ts);

		m_Framebuffer->Unbind();
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

					if (ImGui::MenuItem("Exit")) Application::Get().Close();
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}


			ImGui::Begin("Settings");

			auto stats = Renderer2D::GetStats();
			ImGui::Text("Renderer2D Stats: ");
			ImGui::Text("Draw Calls: %d ", stats.DrawCalls);
			ImGui::Text("Quads: %d ", stats.QuadCount);
			ImGui::Text("Vertices: %d ", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d ", stats.GetTotalIndexCount());

			if(m_SquareEntity)
			{
				ImGui::Separator();
				ImGui::Text("%s", m_SquareEntity.GetComponent<TagComponent>().Tag.c_str());

				auto& squareColor = m_SquareEntity.GetComponent<SpriteRendererComponent>().Color;
				ImGui::ColorEdit4("Square Color", glm::value_ptr(squareColor));
				ImGui::Separator();
			}

			ImGui::DragFloat3("Camera Transform", glm::value_ptr(m_MainCamera.GetComponent<TransformComponent>().Transform[3]));

			if (ImGui::Checkbox("Camera A", &m_PrimaryCamera))
			{
				m_MainCamera.GetComponent<CameraComponent>().Primary = m_PrimaryCamera;
				m_SecondCamera.GetComponent<CameraComponent>().Primary = !m_PrimaryCamera;
			}

			{
				auto& camera = m_SecondCamera.GetComponent<CameraComponent>().Camera;
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Second Camera Ortho Size", &orthoSize))
					camera.SetOrthographicSize(orthoSize);
			}

			ImGui::End();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
			ImGui::Begin("Viewport");

			// 判断当前悬停窗口，以处理事件
			m_ViewportFocused = ImGui::IsWindowFocused();
			m_ViewportHovered = ImGui::IsWindowHovered();
			Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

			// 获取控件视口
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize))
				m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

			uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1), ImVec2(1, 0));
			ImGui::End();
			ImGui::PopStyleVar();

			ImGui::End();
		}
		else
		{
			ImGui::Begin("Settings");

			auto stats = Renderer2D::GetStats();
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

	void EditorLayer::OnEvent(Event& event)
	{
		m_CameraController.OnEvent(event);
	}
}
