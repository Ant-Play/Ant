#pragma once
#include "Ant/Core/PlatformDetection.h"

#include <memory>

namespace Hazel {

	void InitializeCore();
	void ShutdownCore();

}


#ifdef ANT_DEBUG
#if defined(ANT_PLATFORM_WINDOWS)
#define ANT_DEBUGBREAK() __debugbreak();
#elif defined(ANT_PLATFORM_LINUX)
#include <signal.h>
#define ANT_DEBUGBREAK() __debugbreak();
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define ANT_ENABLE_ASSERTS
#else
#define ANT_DEBUGBREAK()
#endif // ANT_DEBUG

#define ANT_EXPAND_MACRO(x) x
#define ANT_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define ANT_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#include <memory>

namespace Ant {
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}

#include "Ant/Core/Log.h"
#include "Ant/Core/Assert.h"
