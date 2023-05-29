#include "antpch.h"
#include "PhysXDebugger.h"

#include "Ant/Physics/3D/PhysXInternal.h"

namespace Ant{

	struct PhysXData
	{
		physx::PxPvd* Debugger;
		physx::PxPvdTransport* Transport;
	};

	static PhysXData* s_Data = nullptr;

#ifdef ANT_DEBUG

	void PhysXDebugger::Initialize()
	{
		s_Data = anew PhysXData();

		s_Data->Debugger = PxCreatePvd(PhysXInternal::GetFoundation());
		ANT_CORE_ASSERT(s_Data->Debugger, "PxCreatePvd failed");
	}

	void PhysXDebugger::Shutdown()
	{
		s_Data->Debugger->release();
		// TODO adelete
		delete s_Data;
		s_Data = nullptr;
	}

	void PhysXDebugger::StartDebugging(const std::string& filepath, bool networkDebugging /*= false*/)
	{
		StopDebugging();

		if (!networkDebugging)
		{
			s_Data->Transport = physx::PxDefaultPvdFileTransportCreate((filepath + ".pxd2").c_str());
			s_Data->Debugger->connect(*s_Data->Transport, physx::PxPvdInstrumentationFlag::eALL);
		}
		else
		{
			s_Data->Transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 1000);
			s_Data->Debugger->connect(*s_Data->Transport, physx::PxPvdInstrumentationFlag::eALL);
		}
	}

	bool PhysXDebugger::IsDebugging()
	{
		return s_Data->Debugger->isConnected();
	}

	void PhysXDebugger::StopDebugging()
	{
		if (!s_Data->Debugger->isConnected())
			return;

		s_Data->Debugger->disconnect();
		s_Data->Debugger->release();
		s_Data->Transport->release();
	}

	physx::PxPvd* PhysXDebugger::GetDebugger()
	{
		return s_Data->Debugger;
	}

#else

	void PhysXDebugger::Initialize() {}
	void PhysXDebugger::Shutdown() {}
	void PhysXDebugger::StartDebugging(const std::string& filepath, bool networkDebugging /*= false*/) {}
	bool PhysXDebugger::IsDebugging() { return false; }
	void PhysXDebugger::StopDebugging() {}
	physx::PxPvd* PhysXDebugger::GetDebugger() { return nullptr; }

#endif
}