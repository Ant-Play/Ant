#pragma once
#define ANT_PLATFORM_WINDOWS
#ifdef ANT_PLATFORM_WINDOWS

extern Ant::Application* Ant::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

int main(int argc, char** argv)
{
	while (g_ApplicationRunning)
	{
		Ant::InitializeCore();
		auto app = Ant::CreateApplication(argc, argv);
		ANT_CORE_ASSERT(app, "Client Application is null!");
		app->Run();
		delete app;
		Ant::ShutdownCore();
	}
}
#else
	#error Ant only supports Windows
#endif // ANT_PLATFORM_WINDOWS
