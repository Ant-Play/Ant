#pragma once
#include "Ant/Core/Core.h"
#include "Ant/Events/ApplicationEvent.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Ant/Core/LayerStack.h"
#include "Ant/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Ant{

	// 应用程序类，作为所有应用程序的基类
	class Application
	{
	public:
		Application();
		virtual ~Application();

		// 事件处理函数
		void OnEvent(Event& e);

		// 推入新图层
		void PushLayer(Layer* layer);
		// 推入新覆盖层
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		
		void Close();

		inline static Application& Get() { return *s_Instance; }
	private:
		void Run();
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}

