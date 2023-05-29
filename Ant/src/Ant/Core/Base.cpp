#include "antpch.h"
#include "Base.h"

#include "Log.h"
#include "Memory.h"

#define ANT_BUILD_ID "v0.1a"

namespace Ant{

	void InitializeCore()
	{
		Allocator::Init();
		Log::Init();

		ANT_CORE_TRACE_TAG("Core", "Ant Engine {}", ANT_BUILD_ID);
		ANT_CORE_TRACE_TAG("Core", "Initializing...");
	}

	void ShutdownCore()
	{
		ANT_CORE_TRACE_TAG("Core", "Shutting down...");
		Log::Shutdown();
	}
}


