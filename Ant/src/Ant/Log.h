#pragma once
#include <memory>

#include "Core.h"
#include "spdlog/spdlog.h"



//--------------»’÷æø‚---------------------
namespace Ant{
	class ANT_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define ANT_CORE_TRACE(...)		::Ant::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define ANT_CORE_INFO(...)		::Ant::Log::GetCoreLogger()->info(__VA_ARGS__);
#define ANT_CORE_WARN(...)		::Ant::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define ANT_CORE_ERROR(...)		::Ant::Log::GetCoreLogger()->error(__VA_ARGS__);
#define ANT_CORE_FATAL(...)		::Ant::Log::GetCoreLogger()->fatal(__VA_ARGS__);

// Client log macros
#define ANT_TRACE(...)		::Ant::Log::GetClientLogger()->trace(__VA_ARGS__);
#define ANT_INFO(...)		::Ant::Log::GetClientLogger()->info(__VA_ARGS__);
#define ANT_WARN(...)		::Ant::Log::GetClientLogger()->warn(__VA_ARGS__);
#define ANT_ERROR(...)		::Ant::Log::GetClientLogger()->error(__VA_ARGS__);
#define ANT_FATAL(...)		::Ant::Log::GetClientLogger()->fatal(__VA_ARGS__);
