#pragma once
#include "Ant/Core/Base.h"
#include "Ant/Core/Application.h"

#ifdef ANT_PLATFORM_WINDOWS

extern Ant::Application* Ant::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	Ant::Log::Init();

	ANT_PROFILE_BEGIN_SESSION("Startup", "AntProfile-Startup.json");
	auto app = Ant::CreateApplication({ argc, argv });
	ANT_PROFILE_END_SESSION();

	ANT_PROFILE_BEGIN_SESSION("Runtime", "AntProfile-Runtime.json");
	app->Run();
	ANT_PROFILE_END_SESSION();

	ANT_PROFILE_BEGIN_SESSION("Shutdown", "AntProfile-Shutdown.json");
	delete app;
	ANT_PROFILE_END_SESSION();

	return 0;
}
#else
	#error Ant only supports Windows
#endif // ANT_PLATFORM_WINDOWS
