#pragma once
#include "Core.h"
#include "Ant/Events/Event.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Ant/Events/ApplicationEvent.h"
#include "Window.h"
#include "Ant/LayerStack.h"
#include "Ant/ImGui/ImGuiLayer.h"
#include "Ant/Renderer/Shader.h"


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

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }
	private:
		bool OnWindowClosed(WindowCloseEvent& e);

		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;

		unsigned int m_VertexArray, m_VertexBuffer, m_IndexBuffer;
		Scope<Shader> m_Shader;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}

