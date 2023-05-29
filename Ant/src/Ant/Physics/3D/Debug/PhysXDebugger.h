#pragma once

#include <PhysX/PxPhysicsAPI.h>

namespace Ant {

	class PhysXDebugger
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void StartDebugging(const std::string& filepath, bool networkDebugging = false);
		static bool IsDebugging();
		static void StopDebugging();

		static physx::PxPvd* GetDebugger();
	};

}