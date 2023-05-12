#pragma once
#include "Ant/Core/Base.h"
#include "Ant/Core/Application.h"
#include "Ant/Debug/Instrumentor.h"

#ifdef ANT_PLATFORM_WINDOWS

extern Ant::Application* Ant::CreateApplication();

int main(int argc, char** argv)
{
	Ant::InitializeCore();
	auto app = Ant::CreateApplication();
	app->Run();
	delete app;
	Ant::ShutdownCore();

	system("pause");
	//return 0;
}
#else
	#error Ant only supports Windows
#endif // ANT_PLATFORM_WINDOWS
