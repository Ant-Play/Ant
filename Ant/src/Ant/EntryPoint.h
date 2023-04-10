#pragma once

#ifdef ANT_PLATFORM_WINDOWS

extern Ant::Application* Ant::CreateApplication();

int main(int argc, char** argv)
{
	Ant::Log::Init();
	ANT_CORE_WARN("Initialized Log!");
	int a = 5;
	ANT_INFO("Hello! Var={0}", a);


	printf("Ant Engine\n");
	auto app = Ant::CreateApplication();
	app->Run();
	delete app;
	return 0;
}
#endif // ANT_PLATFORM_WINDOWS
