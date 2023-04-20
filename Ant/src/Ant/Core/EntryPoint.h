#pragma once
#include "Ant/Core/Application.h"

#ifdef ANT_PLATFORM_WINDOWS

// ����Ӧ�ó���ʵ��
extern Ant::Application* Ant::CreateApplication();


//�������н����
int main(int argc, char** argv)
{
	Ant::Log::Init();

	//���ܷ���
	ANT_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
	auto app = Ant::CreateApplication();
	ANT_PROFILE_END_SESSION();

	ANT_PROFILE_BEGIN_SESSION("Runtime", "HazelProfile-Runtime.json");
	app->Run();
	ANT_PROFILE_END_SESSION();

	ANT_PROFILE_BEGIN_SESSION("Shutdown", "HazelProfile-Shutdown.json");
	delete app;
	ANT_PROFILE_END_SESSION();


	return 0;
}
#endif // ANT_PLATFORM_WINDOWS
