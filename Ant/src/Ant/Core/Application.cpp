#include "antpch.h"
#include "Ant/Core/Application.h"
#include "Ant/Core/Log.h"
#include "Ant/Core/Inputs.h"

#include "Ant/Renderer/Renderer.h"

#include "Ant/Utils/PlatformUtils.h"

#include "Ant/Scripts/ScriptsEngine.h"

#include <GLFW/glfw3.h>

namespace Ant {
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		ANT_PROFILE_FUNCTION(); // 性能分析

		ANT_CORE_ASSERT(!s_Instance, "Application already exists!"); // 断言，确保实例不存在
		s_Instance = this;
		// Set working directory here
		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		m_Window = Window::Create(WindowProps(m_Specification.Name)); // 创建窗口

		m_Window->SetEventCallback(ANT_BIND_EVENT_FN(Application::OnEvent)); // 设置事件回调函数
		m_Window->SetVSync(false); // 关闭垂直同步

		Renderer::Init(); // 初始化渲染器
		ScriptsEngine::Init();

		m_ImGuiLayer = new ImGuiLayer(); // 创建ImGui层
		PushOverlay(m_ImGuiLayer); // 将ImGui层推入图层栈
	}

	Application::~Application()
	{
		ANT_PROFILE_FUNCTION(); // 性能分析

		ScriptsEngine::Shutdown();
		Renderer::Shutdown(); // 关闭渲染器
	}

	void Application::PushLayer(Layer* layer)
	{
		ANT_PROFILE_FUNCTION(); // 性能分析

		m_LayerStack.PushLayer(layer); // 将图层推入图层栈
		layer->OnAttach(); // 调用图层的OnAttach函数
	}

	void Application::PushOverlay(Layer* layer)
	{
		ANT_PROFILE_FUNCTION(); // 性能分析

		m_LayerStack.PushOverlay(layer); // 将覆盖层推入图层栈
		layer->OnAttach(); // 调用覆盖层的OnAttach函数
	}

	// 关闭应用程序
	void Application::Close()
	{
		m_Running = false;
	}

	// 处理事件
	void Application::OnEvent(Event& e)
	{
		ANT_PROFILE_FUNCTION();

		EventDispatcher dispathcher(e);
		dispathcher.Dispatch<WindowCloseEvent>(ANT_BIND_EVENT_FN(Application::OnWindowClosed));
		dispathcher.Dispatch<WindowResizeEvent>(ANT_BIND_EVENT_FN(Application::OnWindowResized));

		//事务响应从后往前
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	// 应用程序运行循环
	void Application::Run()
	{
		ANT_PROFILE_FUNCTION();

		while (m_Running)
		{
			ANT_PROFILE_SCOPE("RunLoop");

			float time = Time::GetTime(); // Platform::GetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				{
					ANT_PROFILE_SCOPE("LayerStack OnUpdate");

					//绘制图层从前往后
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(timestep);
				}
			}
			m_ImGuiLayer->Begin();
			{
				ANT_PROFILE_SCOPE("LayerStack OnImGuiRender");
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}
			m_ImGuiLayer->End();

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResized(WindowResizeEvent& e)
	{
		ANT_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
	}
}