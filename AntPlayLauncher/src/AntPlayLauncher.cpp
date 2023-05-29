#include <Ant/EntryPoint.h>
#include "LauncherLayer.h"

class AntPlayLauncherApplication : public Ant::Application
{
public:
	AntPlayLauncherApplication(const Ant::ApplicationSpecification& specification)
		: Application(specification), m_UserPreferences(Ant::Ref<Ant::UserPreferences>::Create())
	{
	}

	virtual void OnInit() override
	{
		std::filesystem::path persistentStoragePath = Ant::FileSystem::GetPersistentStoragePath();

		// User Preferences
		{
			Ant::UserPreferencesSerializer serializer(m_UserPreferences);
			if (!std::filesystem::exists(persistentStoragePath / "UserPreferences.yaml"))
				serializer.Serialize(persistentStoragePath / "UserPreferences.yaml");
			else
				serializer.Deserialize(persistentStoragePath / "UserPreferences.yaml");
		}

		Ant::LauncherProperties launcherProperties;
		launcherProperties.UserPreferences = m_UserPreferences;
		launcherProperties.ProjectOpenedCallback = std::bind(&AntPlayLauncherApplication::OnProjectOpened, this, std::placeholders::_1);

		// Installation Path
		{
			if (Ant::FileSystem::HasEnvironmentVariable("ANT_DIR"))
				launcherProperties.InstallPath = Ant::FileSystem::GetEnvironmentVariable("ANT_DIR");
		}

		SetShowStats(false);
		PushLayer(new Ant::LauncherLayer(launcherProperties));
	}

private:
	void OnProjectOpened(std::string projectPath)
	{
		std::string antDir = Ant::FileSystem::GetEnvironmentVariable("ANT_DIR");
		std::replace(antDir.begin(), antDir.end(), '\\', '/');

		std::string antplayWorkingDirectory = antDir + "/AntPlay";

#ifdef ANT_DEBUG
		antDir += "bin/Debug-windows-x86_64/AntPlay";
#else
		antDir += "bin/Release-windows-x86_64/AntPlay";
#endif

		std::string antplayExe = antDir + "/AntPlay.exe";
		std::string commandLine = antplayExe + " " + projectPath;

		PROCESS_INFORMATION processInfo;
		STARTUPINFOA startupInfo;
		ZeroMemory(&startupInfo, sizeof(STARTUPINFOA));
		startupInfo.cb = sizeof(startupInfo);

		bool result = CreateProcessA(antplayExe.c_str(), commandLine.data(), NULL, NULL, FALSE, DETACHED_PROCESS, NULL, antplayWorkingDirectory.c_str(), &startupInfo, &processInfo);
		if (result)
		{
			CloseHandle(processInfo.hThread);
			CloseHandle(processInfo.hProcess);
		}
	}

private:
	Ant::Ref<Ant::UserPreferences> m_UserPreferences;
};

Ant::Application* Ant::CreateApplication(int argc, char** argv)
{
	Ant::ApplicationSpecification specification;
	specification.Name = "AntPlay Launcher";
	specification.WindowWidth = 1280;
	specification.WindowHeight = 720;
	specification.VSync = true;
	specification.StartMaximized = false;
	specification.Resizable = false;
	specification.WorkingDirectory = FileSystem::HasEnvironmentVariable("ANT_DIR") ? FileSystem::GetEnvironmentVariable("ANT_DIR") + "/AntPlay" : "../AntPlay";

	return new AntPlayLauncherApplication(specification);
}
