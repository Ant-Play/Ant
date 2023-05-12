#pragma once

#include "Ant/Core/Base.h"
#include "Ant/Core/Timestep.h"
#include "Ant/Core/Window.h"
#include "Ant/Core/LayerStack.h"

#include "Ant/Core/Events/ApplicationEvent.h"

#include "Ant/ImGui/ImGuiLayer.h"

namespace Ant {

	struct ApplicationProps
	{
		std::string Name;
		uint32_t WindowWidth, WindowHeight;
	};

	class Application
	{
	public:
		Application(const ApplicationProps& props = { "Ant Engine", 1280, 720 });
		virtual ~Application();

		void Run();

		virtual void OnInit() {}
		virtual void OnShutdown() {}
		virtual void OnUpdate(Timestep ts) {}

		virtual void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void RenderImGui();

		std::string OpenFile(const char* filter = "All\0*.*\0") const;
		std::string SaveFile(const char* filter = "All\0*.*\0") const;

		inline Window& GetWindow() { return *m_Window; }

		static inline Application& Get() { return *s_Instance; }

		float GetTime() const; // TODO: This should be in "Platform"

		static const char* GetConfigurationName();
		static const char* GetPlatformName();
	private:
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnWindowClose(WindowCloseEvent& e);
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true, m_Minimized = false;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
		Timestep m_TimeStep;

		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
	};

	// Implemented by CLIENT
	Application* CreateApplication();
}