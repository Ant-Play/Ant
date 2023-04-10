#pragma once

#ifdef ANT_PLATFORM_WINDOWS
	#ifdef ANT_BUILD_DLL
		#define ANT_API _declspec(dllexport)
	#else
		#define ANT_API _declspec(dllimport)
	#endif // DEBUG
#else
	#error Ant only support Windows
#endif // HZ_PLATFORM_WINDOWS