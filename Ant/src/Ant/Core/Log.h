#pragma once

#include "Base.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include <glm/glm.hpp>



//--------------»’÷æø‚---------------------
namespace Ant{
	class Log
	{
	public:
		static void Init();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

template<typename OStream>
OStream& operator<<(OStream& os, const glm::vec3& vec)
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ')';
}

template<typename OStream>
OStream& operator<<(OStream& os, const glm::vec4& vec)
{
	return os << '(' << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ')';
}

// Core Logging Macros
#define ANT_CORE_TRACE(...)    ::Ant::Log::GetCoreLogger()->trace(__VA_ARGS__);
#define ANT_CORE_INFO(...)     ::Ant::Log::GetCoreLogger()->info(__VA_ARGS__);
#define ANT_CORE_WARN(...)     ::Ant::Log::GetCoreLogger()->warn(__VA_ARGS__);
#define ANT_CORE_ERROR(...)    ::Ant::Log::GetCoreLogger()->error(__VA_ARGS__);
#define ANT_CORE_FATAL(...)    ::Ant::Log::GetCoreLogger()->critical(__VA_ARGS__);

// Client Logging Macros
#define ANT_TRACE(...)		::Ant::Log::GetClientLogger()->trace(__VA_ARGS__);
#define ANT_INFO(...)		::Ant::Log::GetClientLogger()->info(__VA_ARGS__);
#define ANT_WARN(...)		::Ant::Log::GetClientLogger()->warn(__VA_ARGS__);
#define ANT_ERROR(...)		::Ant::Log::GetClientLogger()->error(__VA_ARGS__);
#define ANT_FATAL(...)		::Ant::Log::GetClientLogger()->critical(__VA_ARGS__);
