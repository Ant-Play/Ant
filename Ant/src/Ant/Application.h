#pragma once
#include "Core.h"
#include "Ant/Events/Event.h"
#include "Platform/WindowsWindow.h"
#include "Ant/Events/ApplicationEvent.h"
#include "Window.h"
namespace Ant{
	class ANT_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		bool OnWindowClosed(WindowCloseEvent& e);

	private:

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}

