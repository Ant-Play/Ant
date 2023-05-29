#include "antpch.h"
#include "PhysXInternal.h"
#include "CookingFactory.h"
#include "PhysicsLayer.h"

#include "Ant/Math/Math.h"

#include "Debug/PhysXDebugger.h"

namespace Ant {

	struct PhysXData
	{
		physx::PxFoundation* PhysXFoundation;
		physx::PxDefaultCpuDispatcher* PhysXCPUDispatcher;
		physx::PxPhysics* PhysXSDK;

		physx::PxDefaultAllocator Allocator;
		PhysicsErrorCallback ErrorCallback;
		PhysicsAssertHandler AssertHandler;

		std::string LastErrorMessage = "";
	};

	static PhysXData* s_PhysXData;

	void PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		const char* errorMessage = NULL;

		switch (code)
		{
			case physx::PxErrorCode::eNO_ERROR:				errorMessage = ""; break;
			case physx::PxErrorCode::eDEBUG_INFO:			errorMessage = "Info"; break;
			case physx::PxErrorCode::eDEBUG_WARNING:		errorMessage = "Warning"; break;
			case physx::PxErrorCode::eINVALID_PARAMETER:	errorMessage = "Invalid Parameter"; break;
			case physx::PxErrorCode::eINVALID_OPERATION:	errorMessage = "Invalid Operation"; break;
			case physx::PxErrorCode::eOUT_OF_MEMORY:		errorMessage = "Out Of Memory"; break;
			case physx::PxErrorCode::eINTERNAL_ERROR:		errorMessage = "Internal Error"; break;
			case physx::PxErrorCode::eABORT:				errorMessage = "Abort"; break;
			case physx::PxErrorCode::ePERF_WARNING:			errorMessage = "Performance Warning"; break;
			case physx::PxErrorCode::eMASK_ALL:				errorMessage = "Unknown Error"; break;
		}

		s_PhysXData->LastErrorMessage = fmt::format("{0}: {1}", errorMessage, message);

		switch (code)
		{
			case physx::PxErrorCode::eNO_ERROR:
			case physx::PxErrorCode::eDEBUG_INFO:
				ANT_CORE_INFO_TAG("Physics", "{0} at {1} ({2})", s_PhysXData->LastErrorMessage, file, line);
				break;
			case physx::PxErrorCode::eDEBUG_WARNING:
			case physx::PxErrorCode::ePERF_WARNING:
				ANT_CORE_WARN_TAG("Physics", "{0} at {1} ({2})", s_PhysXData->LastErrorMessage, file, line);
				break;
			case physx::PxErrorCode::eINVALID_PARAMETER:
			case physx::PxErrorCode::eINVALID_OPERATION:
			case physx::PxErrorCode::eOUT_OF_MEMORY:
			case physx::PxErrorCode::eINTERNAL_ERROR:
				ANT_CORE_ERROR_TAG("Physics", "{0} at {1} ({2})", s_PhysXData->LastErrorMessage, file, line);
				ANT_CORE_ASSERT(false);
				break;
			case physx::PxErrorCode::eABORT:
			case physx::PxErrorCode::eMASK_ALL:
				ANT_CORE_FATAL_TAG("Physics", "{0} at {1} ({2})", s_PhysXData->LastErrorMessage, file, line);
				ANT_CORE_ASSERT(false);
				break;
		}
	}

	void PhysicsAssertHandler::operator()(const char* exp, const char* file, int line, bool& ignore)
	{
		ANT_CORE_ERROR_TAG("Physics", "{0}:{1} - {2}", file, line, exp);
		ANT_CORE_ASSERT(false);
	}

	void PhysXInternal::Initialize()
	{
		ANT_CORE_ASSERT(!s_PhysXData, "Trying to initialize the PhysX SDK multiple times!");

		s_PhysXData = anew PhysXData();

		s_PhysXData->PhysXFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_PhysXData->Allocator, s_PhysXData->ErrorCallback);
		ANT_CORE_ASSERT(s_PhysXData->PhysXFoundation, "PxCreateFoundation failed.");

		physx::PxTolerancesScale scale = physx::PxTolerancesScale();
		scale.length = 1.0f;
		scale.speed = 100.0f;

		PhysXDebugger::Initialize();

#ifdef ANT_DEBUG
		static bool s_TrackMemoryAllocations = true;
#else
		static bool s_TrackMemoryAllocations = false;
#endif

		s_PhysXData->PhysXSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *s_PhysXData->PhysXFoundation, scale, s_TrackMemoryAllocations, PhysXDebugger::GetDebugger());
		ANT_CORE_ASSERT(s_PhysXData->PhysXSDK, "PxCreatePhysics failed.");

		bool extentionsLoaded = PxInitExtensions(*s_PhysXData->PhysXSDK, PhysXDebugger::GetDebugger());
		ANT_CORE_ASSERT(extentionsLoaded, "Failed to initialize PhysX Extensions.");

		s_PhysXData->PhysXCPUDispatcher = physx::PxDefaultCpuDispatcherCreate(1);

		CookingFactory::Initialize();

		PxSetAssertHandler(s_PhysXData->AssertHandler);
	}

	void PhysXInternal::Shutdown()
	{
		CookingFactory::Shutdown();

		s_PhysXData->PhysXCPUDispatcher->release();
		s_PhysXData->PhysXCPUDispatcher = nullptr;

		PxCloseExtensions();

		PhysXDebugger::StopDebugging();

		s_PhysXData->PhysXSDK->release();
		s_PhysXData->PhysXSDK = nullptr;

		PhysXDebugger::Shutdown();

		s_PhysXData->PhysXFoundation->release();
		s_PhysXData->PhysXFoundation = nullptr;

		// TODO adelete
		delete s_PhysXData;
		s_PhysXData = nullptr;
	}

	physx::PxFoundation& PhysXInternal::GetFoundation() { return *s_PhysXData->PhysXFoundation; }
	physx::PxPhysics& PhysXInternal::GetPhysXSDK() { return *s_PhysXData->PhysXSDK; }
	physx::PxCpuDispatcher* PhysXInternal::GetCPUDispatcher() { return s_PhysXData->PhysXCPUDispatcher; }
	physx::PxDefaultAllocator& PhysXInternal::GetAllocator() { return s_PhysXData->Allocator; }
	const std::string& PhysXInternal::GetLastErrorMessage() { return s_PhysXData->LastErrorMessage; }

	physx::PxFilterFlags PhysXInternal::FilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
	{
		if (filterData0.word3 == filterData1.word3)
		{
			const auto& layer = PhysicsLayerManager::GetLayer(filterData0.word3);

			if (!layer.CollidesWithSelf)
				return physx::PxFilterFlag::eSUPPRESS;
		}

		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return physx::PxFilterFlag::eDEFAULT;
		}

		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		if (filterData0.word2 == (uint32_t)CollisionDetectionType::Continuous || filterData1.word2 == (uint32_t)CollisionDetectionType::Continuous)
		{
			pairFlags |= physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;
			pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;
		}

		if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
			return physx::PxFilterFlag::eDEFAULT;
		}

		return physx::PxFilterFlag::eSUPPRESS;
	}

}