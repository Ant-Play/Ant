#pragma once
#include "Ant/Core/PlatformDetection.h"

//#ifdef ANT_PLATFORM_WINDOWS
//#if ANT_DYNAMIC_LINK
//	#ifdef ANT_BUILD_DLL
//		#define ANT_API _declspec(dllexport)
//	#else
//		#define ANT_API _declspec(dllimport)
//	#endif // DEBUG
//#else
//	#define ANT_API
//#endif
//#else
//	#error Ant only support Windows
//#endif // ANT_PLATFORM_WINDOWS

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

#ifdef ANT_ENABLE_ASSERTS
	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define ANT_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { ANT##type##ERROR(msg, __VA_ARGS__); ANT_DEBUGBREAK(); } }
	#define ANT_INTERNAL_ASSERT_WITH_MSG(type, check, ...) ANT_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define ANT_INTERNAL_ASSERT_NO_MSG(type, check) ANT_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", ANT_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define ANT_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define ANT_INTERNAL_ASSERT_GET_MACRO(...) ANT_EXPAND_MACRO( ANT_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, ANT_INTERNAL_ASSERT_WITH_MSG, ANT_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define ANT_ASSERT(...) ANT_EXPAND_MACRO( ANT_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define ANT_CORE_ASSERT(...) ANT_EXPAND_MACRO( ANT_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define ANT_ASSERT(x, ...)
	#define ANT_CORE_ASSERT(x, ...)
#endif // ANT_ENABLE_ASSERTS

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
