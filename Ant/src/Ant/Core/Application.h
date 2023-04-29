#pragma once

#include "Ant/Core/Base.h"

#include "Ant/Core/Window.h"
#include "Ant/Core/LayerStack.h"
#include "Ant/Events/Event.h"
#include "Ant/Events/ApplicationEvent.h"

#include "Ant/Core/Timestep.h"

#include "Ant/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Ant{

	// Ӧ�ó����࣬��Ϊ����Ӧ�ó���Ļ���
	class Application
	{
	public:
		Application(const std::string& name = "Ant Engine");
		virtual ~Application();

		// �¼�������
		void OnEvent(Event& e);

		// ������ͼ��
		void PushLayer(Layer* layer);
		// �����¸��ǲ�
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

