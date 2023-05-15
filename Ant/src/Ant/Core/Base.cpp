#include "antpch.h"
#include "Base.h"
#include "Log.h"

#define ANT_BUILD_ID "v0.1a"

namespace Ant{

	void InitializeCore()
	{
		Log::Init();

		ANT_CORE_TRACE("Ant Engine: {}", ANT_BUILD_ID);
		ANT_CORE_TRACE("Initializing...");
	}

	void ShutdownCore()
	{
		ANT_CORE_TRACE("Shutting down...");
		Log::Shutdown();
	}
}


