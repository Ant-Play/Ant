#include "antpch.h"
#include "Ant/Core/Application.h"
#include "Ant/Core/Log.h"

#include "Ant/Renderer/Renderer.h"

#include "Ant/Utils/PlatformUtils.h"
#include "Ant/Scripts/ScriptEngine.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>
#include <commdlg.h>

#include <glad/glad.h>

namespace Ant {
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationProps& props)
	{
		s_Instance = this;

		std::filesystem::current_path("D:/career/graphic/workspace/Ant-dev/AntPlay");

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(props.Name, props.WindowWidth, props.WindowHeight)));
		m_Window->SetEventCallback(ANT_BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		m_ImGuiLayer = new ImGuiLayer("ImGui");
		PushOverlay(m_ImGuiLayer);

		ScriptEngine::Init("assets/scripts/ExampleApp.dll");

		Renderer::Init();
		Renderer::WaitAndRender();
	}

	Application::~Application()
	{
		ScriptEngine::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::RenderImGui()
	{
		m_ImGuiLayer->Begin();

		ImGui::Begin("Renderer");
		auto& caps = RendererAPI::GetCapabilities();
		ImGui::Text("Vendor: %s", caps.Vendor.c_str());
		ImGui::Text("Renderer: %s", caps.Renderer.c_str());
		ImGui::Text("Version: %s", caps.Version.c_str());
		ImGui::Text("Frame Time: %.2fms\n", m_TimeStep.GetMilliseconds());
		ImGui::End();

		for (Layer* layer : m_LayerStack)
			layer->OnImGuiRender();

		m_ImGuiLayer->End();
	}

	void Application::Run()
	{
		OnInit();
		while (m_Running)
		{
			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(m_TimeStep);

				// Render ImGui on render thread
				Application* app = this;
				Renderer::Submit([app]() { app->RenderImGui(); });

				Renderer::WaitAndRender();
			}
			m_Window->OnUpdate();

			float time = GetTime();
			m_TimeStep = time - m_LastFrameTime;
			m_LastFrameTime = time;
		}
		OnShutdown();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>(ANT_BIND_EVENT_FN(Application::OnWindowResize));
		dispatcher.Dispatch<WindowCloseEvent>(ANT_BIND_EVENT_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		int width = e.GetWidth(), height = e.GetHeight();
		if (width == 0 || height == 0)
		{
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::Submit([=]() { glViewport(0, 0, width, height); });
		auto& fbs = FramebufferPool::GetGlobal()->GetAll();
		for (auto& fb : fbs)
			fb->Resize(width, height);
		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	std::string Application::OpenFile(const char* filter) const
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}

	std::string Application::SaveFile(const char* filter /*= "All\0*.*\0"*/) const
	{
		OPENFILENAMEA ofn;       // common dialog box structure
		CHAR szFile[260] = { 0 };       // if using TCHAR macros

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)m_Window->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}

	float Application::GetTime() const
	{
		return (float)glfwGetTime();
	}

	const char* Application::GetConfigurationName()
	{
#if defined(ANT_DEBUG)
		return "Debug";
#elif defined(ANT_RELEASE)
		return "Release";
#elif defined(ANT_DIST)
		return "Dist";
#else
	#error Unknown configuration!
#endif
	}

	const char* Application::GetPlatformName()
	{
#if defined(ANT_PLATFORM_WINDOWS)
		return "Windows x64";
#else
	#error Unknown platform!
#endif
	}

}
