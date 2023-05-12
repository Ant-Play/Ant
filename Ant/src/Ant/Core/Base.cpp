#include "antpch.h"
#include "Ant/Core/Base.h"
#include "Ant/Core/Log.h"

#include <Windows.h>

#define ANT_BUILD_ID "v0.1a"

namespace Ant{

	void InitializeCore()
	{
		// Initialize the logger
		Log::Init();
		ANT_CORE_INFO("Initialized Core Log");
		// Initialize the build id
		ANT_CORE_TRACE("Ant Engine: {0}", ANT_BUILD_ID);
		ANT_CORE_TRACE("Initializing...");
	}

	void ShutdownCore()
	{
		ANT_CORE_TRACE("Shutting down...");
	}
}


