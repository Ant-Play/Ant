#include "antpch.h"
#include "Application.h"
#include "Ant/Inputs.h"
#include "Ant/KeyCodes.h"

#include <glad/glad.h>
namespace Ant {

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		ANT_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_Window = Window::Create();
		m_Window->SetEventCallback(ANT_BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application()
	{

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

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispathcher(e);
		dispathcher.Dispatch<WindowCloseEvent>(ANT_BIND_EVENT_FN(Application::OnWindowClosed));

		ANT_CORE_TRACE("{0}", e);

		//事务响应从后往前
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);
			if(e.m_Handled)
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			//绘制图层从前往后
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();
			auto [x, y] = Input::GetMousePosition();
			ANT_CORE_TRACE("{0}, {1}", x, y);

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}
}
