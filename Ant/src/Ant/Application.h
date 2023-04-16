#pragma once
#include "Ant/Core.h"
#include "Ant/Events/Event.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Ant/Events/ApplicationEvent.h"
#include "Ant/Window.h"
#include "Ant/LayerStack.h"
#include "Ant/ImGui/ImGuiLayer.h"
#include "Ant/Core/Timestep.h"

namespace Ant{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClosed(WindowCloseEvent& e);

		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}

