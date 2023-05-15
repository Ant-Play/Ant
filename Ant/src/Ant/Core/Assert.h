#pragma once


#ifdef ANT_DEBUG
#define ANT_ENABLE_ASSERTS
#endif

#ifdef ANT_ENABLE_ASSERTS
#define ANT_ASSERT_NO_MESSAGE(condition) { if(!(condition)) { ANT_ERROR("Assertion Failed"); __debugbreak(); } }
#define ANT_ASSERT_MESSAGE(condition, ...) { if(!(condition)) { ANT_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }

#define ANT_ASSERT_RESOLVE(arg1, arg2, macro, ...) macro
#define ANT_GET_ASSERT_MACRO(...) ANT_EXPAND_VARGS(ANT_ASSERT_RESOLVE(__VA_ARGS__, ANT_ASSERT_MESSAGE, ANT_ASSERT_NO_MESSAGE))

#define ANT_ASSERT(...) ANT_EXPAND_VARGS( ANT_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#define ANT_CORE_ASSERT(...) ANT_EXPAND_VARGS( ANT_GET_ASSERT_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#else
#define ANT_ASSERT(...)
#define ANT_CORE_ASSERT(...)
#endif
