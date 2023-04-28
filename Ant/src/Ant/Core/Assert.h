#pragma once

#include "Ant/Core/Base.h"
#include "Ant/Core/Log.h"
#include <filesystem>

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
