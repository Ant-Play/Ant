#pragma once

#ifdef ANT_PLATFORM_WINDOWS
#if ANT_DYNAMIC_LINK
	#ifdef ANT_BUILD_DLL
		#define ANT_API _declspec(dllexport)
	#else
		#define ANT_API _declspec(dllimport)
	#endif // DEBUG
#else
	#define ANT_API
#endif
#else
	#error Ant only support Windows
#endif // HZ_PLATFORM_WINDOWS

#ifdef HZ_DEBUG
	#define ANT_ENABLE_ASSERTS
#endif // HZ_DEBUG


#ifdef ANT_ENABLE_ASSERTS
	#define ANT_ASSERT(x, ...) { if(!(x)) {ANT_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define ANT_CORE_ASSERT(x, ...) { if(!(x)) { ANT_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); }}
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
