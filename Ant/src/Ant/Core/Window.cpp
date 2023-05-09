#include "antpch.h"
#include "Ant/Core/Window.h"

#ifdef ANT_PLATFORM_WINDOWS
#include "Ant/Platform/Windows/WindowsWindow.h"
#endif

namespace Ant {
	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef ANT_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#else
		ANT_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}
}