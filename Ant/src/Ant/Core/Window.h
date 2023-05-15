#pragma once

#include <functional>

#include "Base.h"
#include "Events/Event.h"
#include "Ant/Renderer/RendererContext.h"

namespace Ant {

	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Ant Engine",
					uint32_t width = 1600,
					uint32_t height = 900)
			: Title(title), Width(width), Height(height)
		{

		}
	};

	// Interface representing a desktop system based Window
	class Window : public RefCounted
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void ProcessEvents() = 0;
		virtual void SwapBuffers() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual std::pair<uint32_t, uint32_t> GetSize() const = 0;
		virtual std::pair<float, float> GetWindowPos() const = 0;

		virtual void Maximize() = 0;


		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual const std::string& GetTitle() const = 0;
		virtual void SetTitle(const std::string& title) = 0;

		virtual void* GetNativeWindow() const = 0;

		virtual Ref<RendererContext> GetRenderContext() = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};
}
