#include "antpch.h"
#include "Ant/Core/Inputs.h"

#ifdef ANT_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsInput.h"
#endif // ANT_PLATFORM_WINDOWS

namespace Ant {

	Scope<Input> Input::s_Instance = Input::Create();

	Scope<Input> Input::Create()
	{
	#ifdef ANT_PLATFORM_WINDOWS
			return CreateScope<WindowsInputs>();
	#else
			ANT_CORE_ASSERT(false, "Unknown platform!");
			return nullptr;
	#endif // ANT_PLATFORM_WINDOWS
	}
}
