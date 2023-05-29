#pragma once

#include "Base.h"
#include "LogCustomFormatters.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include <map>

#define ANT_ASSERT_MESSAGE_BOX !ANT_DIST

#if ANT_ASSERT_MESSAGE_BOX
	#include <Windows.h>
#endif


//--------------»’÷æø‚---------------------
namespace Ant{
	class Log
	{
	public:
		enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};
		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal
		};
		struct TagDetails
		{
			bool Enabled = true;
			Level LevelFilter = Level::Trace;
		};

	public:
		static void Init();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetEditorConsoleLogger() { return s_EditorConsoleLogger; }

		static bool HasTag(const std::string& tag) { return s_EnabledTags.find(tag) != s_EnabledTags.end(); }
		static std::map<std::string, TagDetails>& EnabledTags() { return s_EnabledTags; }

		template<typename... Args>
		static void PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args);

		template<typename... Args>
		static void PrintAssertMessage(Log::Type type, std::string_view prefix, Args&&... args);

	public:
		// Enum utils
		static const char* LevelToString(Level level)
		{
			switch (level)
			{
				case Level::Trace: return "Trace";
				case Level::Info:  return "Info";
				case Level::Warn:  return "Warn";
				case Level::Error: return "Error";
				case Level::Fatal: return "Fatal";
			}
			return "";
		}
		static Level LevelFromString(std::string_view string)
		{
			if (string == "Trace") return Level::Trace;
			if (string == "Info")  return Level::Info;
			if (string == "Warn")  return Level::Warn;
			if (string == "Error") return Level::Error;
			if (string == "Fatal") return Level::Fatal;

			return Level::Trace;
		}

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_EditorConsoleLogger;

		inline static std::map<std::string, TagDetails> s_EnabledTags;
	};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tagged logs (prefer these!)                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core Logging
#define ANT_CORE_TRACE_TAG(tag, ...)  ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Trace, tag, __VA_ARGS__)
#define ANT_CORE_INFO_TAG(tag, ...)   ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Info, tag, __VA_ARGS__)
#define ANT_CORE_WARN_TAG(tag, ...)   ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Warn, tag, __VA_ARGS__)
#define ANT_CORE_ERROR_TAG(tag, ...)  ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Error, tag, __VA_ARGS__)
#define ANT_CORE_FATAL_TAG(tag, ...)  ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Fatal, tag, __VA_ARGS__)

// Client Logging
#define ANT_TRACE_TAG(tag, ...)	::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Trace, tag, __VA_ARGS__)
#define ANT_INFO_TAG(tag, ...)	::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Info, tag, __VA_ARGS__)
#define ANT_WARN_TAG(tag, ...)	::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Warn, tag, __VA_ARGS__)
#define ANT_ERROR_TAG(tag, ...)	::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Error, tag, __VA_ARGS__)
#define ANT_FATAL_TAG(tag, ...)	::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Fatal, tag, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core Logging
#define ANT_CORE_TRACE(...)  ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Trace, "", __VA_ARGS__)
#define ANT_CORE_INFO(...)   ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Info, "", __VA_ARGS__)
#define ANT_CORE_WARN(...)   ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Warn, "", __VA_ARGS__)
#define ANT_CORE_ERROR(...)  ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Error, "", __VA_ARGS__)
#define ANT_CORE_FATAL(...)  ::Ant::Log::PrintMessage(::Ant::Log::Type::Core, ::Ant::Log::Level::Fatal, "", __VA_ARGS__)

// Client Logging
#define ANT_TRACE(...)	 ::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Trace, "", __VA_ARGS__)
#define ANT_INFO(...)	 ::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Info, "", __VA_ARGS__)
#define ANT_WARN(...)	 ::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Warn, "", __VA_ARGS__)
#define ANT_ERROR(...)	 ::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Error, "", __VA_ARGS__)
#define ANT_FATAL(...)	 ::Ant::Log::PrintMessage(::Ant::Log::Type::Client, ::Ant::Log::Level::Fatal, "", __VA_ARGS__)

// Editor Console Logging Macros
#define ANT_CONSOLE_LOG_TRACE(...)    Ant::Log::GetEditorConsoleLogger()->trace(__VA_ARGS__)
#define ANT_CONSOLE_LOG_INFO(...)     Ant::Log::GetEditorConsoleLogger()->info(__VA_ARGS__)
#define ANT_CONSOLE_LOG_WARN(...)     Ant::Log::GetEditorConsoleLogger()->warn(__VA_ARGS__)
#define ANT_CONSOLE_LOG_ERROR(...)    Ant::Log::GetEditorConsoleLogger()->error(__VA_ARGS__)
#define ANT_CONSOLE_LOG_FATAL(...)    Ant::Log::GetEditorConsoleLogger()->critical(__VA_ARGS__)

namespace Ant{

	template<typename... Args>
	void Log::PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args)
	{
		auto detail = s_EnabledTags[std::string(tag)];
		if(detail.Enabled && detail.LevelFilter <= level)
		{
			auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
			std::string logString = tag.empty() ? "{0} {1}" : "[{0}] {1}";
			switch(level)
			{
				case Level::Trace: logger->trace(logString, tag, fmt::format(std::forward<Args>(args)...)); break;
				case Level::Info:  logger->info(logString, tag, fmt::format(std::forward<Args>(args)...)); break;
				case Level::Warn:  logger->warn(logString, tag, fmt::format(std::forward<Args>(args)...)); break;
				case Level::Error: logger->error(logString, tag, fmt::format(std::forward<Args>(args)...)); break;
				case Level::Fatal: logger->critical(logString, tag, fmt::format(std::forward<Args>(args)...)); break;
			}
		}
	}

	template<typename... Args>
	void Log::PrintAssertMessage(Log::Type type, std::string_view prefix, Args&&... args)
	{
		auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
		logger->error("{0}: {1}", prefix, fmt::format(std::forward<Args>(args)...));

#if ANT_ASSERT_MESSAGE_BOX
		std::string message = fmt::format(std::forward<Args>(args)...);
		MessageBoxA(nullptr, message.c_str(), "Ant Assert", MB_OK | MB_ICONERROR);
#endif
	}

	template<>
	inline void Log::PrintAssertMessage(Log::Type type, std::string_view prefix)
	{
		auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
		logger->error("{0}", prefix);

#if ANT_ASSERT_MESSAGE_BOX
		MessageBoxA(nullptr, "No message: (", "Ant Assert", MB_OK | MB_ICONERROR);
#endif
	}
}