#include "antpch.h"
#include "Ant/Window.h"

#ifdef ANT_PLATFORM_WINDOWS
#include "Platform/WindowsWindow.h"
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
