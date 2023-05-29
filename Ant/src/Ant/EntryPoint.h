#pragma once

#include "Ant/Core/Application.h"

#ifdef ANT_PLATFORM_WINDOWS

extern Ant::Application* Ant::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

namespace Ant {

	int Main(int argc, char** argv)
	{
		while (g_ApplicationRunning)
		{
			InitializeCore();
			Application* app = CreateApplication(argc, argv);
			ANT_CORE_ASSERT(app, "Client Application is null");
			app->Run();
			delete app;
			ShutdownCore();
		}

		return 0;
	}
}

#ifdef ANT_DIST

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return Ant::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return Ant::Main(argc, argv);
}

#endif

#endif
