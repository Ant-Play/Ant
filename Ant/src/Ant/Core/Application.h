#pragma once

#include "Ant/Core/Base.h"

#include "Ant/Core/Window.h"
#include "Ant/Core/LayerStack.h"
#include "Ant/Events/Event.h"
#include "Ant/Events/ApplicationEvent.h"

#include "Ant/Core/Timestep.h"

#include "Ant/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Ant {
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			ANT_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Ant Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	// Ӧ�ó����࣬��Ϊ����Ӧ�ó���Ļ���
	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);

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

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }
	private:
		void Run();
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		ApplicationSpecification m_Specification;
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
	Application* CreateApplication(ApplicationCommandLineArgs args);
}
