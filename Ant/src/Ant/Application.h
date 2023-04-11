#pragma once
#include "Core.h"
#include "Ant/Events/Event.h"
#include "Platform/WindowsWindow.h"
#include "Ant/Events/ApplicationEvent.h"
#include "Window.h"
#include "Ant/LayerStack.h"




namespace Ant{
	class ANT_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
	private:
		bool OnWindowClosed(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}

