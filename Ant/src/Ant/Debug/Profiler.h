#pragma once


#define ANT_ENABLE_PROFILING !ANT_DIST

#if ANT_ENABLE_PROFILING 
#include <optick.h>
#endif

#if ANT_ENABLE_PROFILING
#define ANT_PROFILE_FRAME(...)           OPTICK_FRAME(__VA_ARGS__)
#define ANT_PROFILE_FUNC(...)            OPTICK_EVENT(__VA_ARGS__)
#define ANT_PROFILE_TAG(NAME, ...)       OPTICK_TAG(NAME, __VA_ARGS__)
#define ANT_PROFILE_SCOPE_DYNAMIC(NAME)  OPTICK_EVENT_DYNAMIC(NAME)
#define ANT_PROFILE_THREAD(...)          OPTICK_THREAD(__VA_ARGS__)
#else
#define ANT_PROFILE_FRAME(...)
#define ANT_PROFILE_FUNC(...)
#define ANT_PROFILE_TAG(NAME, ...) 
#define ANT_PROFILE_SCOPE_DYNAMIC(NAME)
#define ANT_PROFILE_THREAD(...)
#endif