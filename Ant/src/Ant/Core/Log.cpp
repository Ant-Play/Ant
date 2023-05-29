#include "antpch.h"
#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "Ant/Editor/EditorConsole/EditorConsoleSink.h"

#include <filesystem>

#define ANT_HAS_CONSOLE !ANT_DIST

namespace Ant
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::shared_ptr<spdlog::logger> Log::s_EditorConsoleLogger;

	void Log::Init()
	{
		// Create "logs" directory if doesn't exist
		std::string logsDirectory = "logs";
		if (!std::filesystem::exists(logsDirectory))
			std::filesystem::create_directories(logsDirectory);

		std::vector<spdlog::sink_ptr> antSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/ANT.log", true),
#if ANT_HAS_CONSOLE
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
#endif
		};

		std::vector<spdlog::sink_ptr> appSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/APP.log", true),
#if ANT_HAS_CONSOLE
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
#endif
		};

		std::vector<spdlog::sink_ptr> editorConsoleSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/APP.log", true),
#if ANT_HAS_CONSOLE
			std::make_shared<EditorConsoleSink>(1)
#endif
		};

		antSinks[0]->set_pattern("[%T] [%l] %n: %v");
		appSinks[0]->set_pattern("[%T] [%l] %n: %v");

#if ANT_HAS_CONSOLE
		antSinks[1]->set_pattern("%^[%T] %n: %v%$");
		appSinks[1]->set_pattern("%^[%T] %n: %v%$");
		for (auto sink : editorConsoleSinks)
			sink->set_pattern("%^%v%$");
#endif

		s_CoreLogger = std::make_shared<spdlog::logger>("ANT", antSinks.begin(), antSinks.end());
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", appSinks.begin(), appSinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);

		s_EditorConsoleLogger = std::make_shared<spdlog::logger>("Console", editorConsoleSinks.begin(), editorConsoleSinks.end());
		s_EditorConsoleLogger->set_level(spdlog::level::trace);
	}

	void Log::Shutdown()
	{
		s_EditorConsoleLogger.reset();
		s_ClientLogger.reset();
		s_CoreLogger.reset();
		spdlog::drop_all();
	}
}