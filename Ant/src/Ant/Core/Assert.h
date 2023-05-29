#pragma once

#include "Log.h"

#ifdef ANT_PLATFORM_WINDOWS
	#define ANT_DEBUG_BREAK __debugbreak()
#else
	#define ANT_DEBUG_BREAK
#endif

#ifdef ANT_DEBUG
	#define ANT_ENABLE_ASSERTS
#endif

#define ANT_ENABLE_VERIFY

#ifdef ANT_ENABLE_ASSERTS
	#define ANT_CORE_ASSERT_MESSAGE_INTERNAL(...) ::Ant::Log::PrintAssertMessage(::Ant::Log::Type::Core, "Assertion Failed", __VA_ARGS__)
	#define ANT_ASSERT_MESSAGE_INTERNAL(...) ::Ant::Log::PrintAssertMessage(::Ant::Log::Type::Client, "Assertion Failed", __VA_ARGS__)

	#define ANT_CORE_ASSERT(condition, ...) { if(!(condition)) { ANT_CORE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); ANT_DEBUG_BREAK; } }
	#define ANT_ASSERT(condition, ...) { if(!(condition)) { ANT_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); ANT_DEBUG_BREAK; } }
#else
	#define ANT_CORE_ASSERT(condition, ...)
	#define ANT_ASSERT(condition, ...)
#endif

#ifdef ANT_ENABLE_VERIFY
	#define ANT_CORE_VERIFY_MESSAGE_INTERNAL(...) ::Ant::Log::PrintAssertMessage(::Ant::Log::Type::Core, "Verification Failed", __VA_ARGS__)
	#define ANT_VERIFY_MESSAGE_INTERNAL(...) ::Ant::Log::PrintAssertMessage(::Ant::Log::Type::Client, "Verification Failed", __VA_ARGS__)

	#define ANT_CORE_VERIFY(condition, ...) { if(!(condition)) { ANT_CORE_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); ANT_DEBUG_BREAK; } }
	#define ANT_VERIFY(condition, ...) { if(!(condition)) { ANT_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); ANT_DEBUG_BREAK; } }
#else
	#define ANT_CORE_VERIFY(condition, ...)
	#define ANT_VERIFY(condition, ...)
#endif
