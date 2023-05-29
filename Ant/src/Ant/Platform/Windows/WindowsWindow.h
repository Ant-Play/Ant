#pragma once
#include "Ant/Core/Window.h"
#include "Ant/Renderer/RendererContext.h"
#include "Ant/Platform/Vulkan/VulkanSwapChain.h"

#include <GLFW/glfw3.h>
namespace Ant {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowSpecification& specification);
		virtual ~WindowsWindow();

		virtual void Init() override;
		virtual void ProcessEvents() override;
		virtual void SwapBuffers() override;

		inline uint32_t GetWidth() const override { return m_Data.Width; }
		inline uint32_t GetHeight() const override { return m_Data.Height; }

		virtual std::pair<uint32_t, uint32_t> GetSize() const override { return { m_Data.Width, m_Data.Height }; }
		virtual std::pair<float, float> GetWindowPos() const override;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool enabled) override;
		virtual bool IsVSync() const override;
		virtual void SetResizable(bool resizable) const override;

		virtual void Maximize() override;
		virtual void CenterWindow() override;

		virtual const std::string& GetTitle() const override { return m_Data.Title; }
		virtual void SetTitle(const std::string& title) override;

		inline void* GetNativeWindow() const override { return m_Window; }

		virtual Ref<RendererContext> GetRenderContext() override { return m_RendererContext; }
		virtual VulkanSwapChain& GetSwapChain() override;
	private:
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;
		GLFWcursor* m_ImGuiMouseCursors[9] = { 0 };
		WindowSpecification m_Specification;

		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
		float m_LastFrameTime = 0.0f;

		Ref<RendererContext> m_RendererContext;
		VulkanSwapChain m_SwapChain;
	};
}
