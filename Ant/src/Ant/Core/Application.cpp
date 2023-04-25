#include "antpch.h"
#include "Ant/Core/Application.h"
#include "Ant/Renderer/Renderer.h"


namespace Ant {

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		ANT_PROFILE_FUNCTION(); // ���ܷ���

		ANT_CORE_ASSERT(!s_Instance, "Application already exists!"); // ���ԣ�ȷ��ʵ��������
		s_Instance = this;

		m_Window = Window::Create(); // ��������
		m_Window->SetEventCallback(ANT_BIND_EVENT_FN(Application::OnEvent)); // �����¼��ص�����
		m_Window->SetVSync(false); // �رմ�ֱͬ��

		Renderer::Init(); // ��ʼ����Ⱦ��

		m_ImGuiLayer = new ImGuiLayer(); // ����ImGui��
		PushOverlay(m_ImGuiLayer); // ��ImGui������ͼ��ջ
	}

	Application::~Application()
	{
		ANT_PROFILE_FUNCTION(); // ���ܷ���

		Renderer::Shutdown(); // �ر���Ⱦ��
	}

	void Application::PushLayer(Layer* layer)
	{
		ANT_PROFILE_FUNCTION(); // ���ܷ���

		m_LayerStack.PushLayer(layer); // ��ͼ������ͼ��ջ
		layer->OnAttach(); // ����ͼ���OnAttach����
	}

	void Application::PushOverlay(Layer* layer)
	{
		ANT_PROFILE_FUNCTION(); // ���ܷ���

		m_LayerStack.PushOverlay(layer); // �����ǲ�����ͼ��ջ
		layer->OnAttach(); // ���ø��ǲ��OnAttach����
	}

	// �ر�Ӧ�ó���
	void Application::Close()
	{
		m_Running = false;
	}

	// �����¼�
	void Application::OnEvent(Event& e)
	{
		ANT_PROFILE_FUNCTION();

		EventDispatcher dispathcher(e);
		dispathcher.Dispatch<WindowCloseEvent>(ANT_BIND_EVENT_FN(Application::OnWindowClosed));
		dispathcher.Dispatch<WindowResizeEvent>(ANT_BIND_EVENT_FN(Application::OnWindowResized));

		//������Ӧ�Ӻ���ǰ
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if(e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	// Ӧ�ó�������ѭ��
	void Application::Run()
	{
		ANT_PROFILE_FUNCTION();

		while (m_Running)
		{
			ANT_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime(); // Platform::GetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				{
					ANT_PROFILE_SCOPE("LayerStack OnUpdate");

					//����ͼ���ǰ����
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