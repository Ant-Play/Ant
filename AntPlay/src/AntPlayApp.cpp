
#include "Ant/EntryPoint.h"

#include "EditorLayer.h"
#include "Ant/Utilities/FileSystem.h"

#include <Shlobj.h>

class AntPlayApplication : public Ant::Application
{
public:
	AntPlayApplication(const Ant::ApplicationSpecification& specification, std::string_view projectPath)
		: Application(specification), m_ProjectPath(projectPath), m_UserPreferences(Ant::Ref<Ant::UserPreferences>::Create())
	{
		if (projectPath.empty())
			m_ProjectPath = "SandboxProject/Sandbox.aproj";
	}

	virtual void OnInit() override
	{
		// Persistent Storage
		{
			PWSTR roamingFilePath;
			HRESULT result = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, NULL, &roamingFilePath);
			ANT_CORE_ASSERT(result == S_OK);
			std::wstring filepath = roamingFilePath;
			std::replace(filepath.begin(), filepath.end(), L'\\', L'/');
			m_PersistentStoragePath = filepath + L"/AntPlay";

			if (!std::filesystem::exists(m_PersistentStoragePath))
				std::filesystem::create_directory(m_PersistentStoragePath);
		}

		// User Preferences
		{
			Ant::UserPreferencesSerializer serializer(m_UserPreferences);
			if (!std::filesystem::exists(m_PersistentStoragePath / "UserPreferences.yaml"))
				serializer.Serialize(m_PersistentStoragePath / "UserPreferences.yaml");
			else
				serializer.Deserialize(m_PersistentStoragePath / "UserPreferences.yaml");

			if (!m_ProjectPath.empty())
				m_UserPreferences->StartupProject = m_ProjectPath;
			else if (!m_UserPreferences->StartupProject.empty())
				m_ProjectPath = m_UserPreferences->StartupProject;
		}

		// Update the ANT_DIR environment variable every time we launch
		{
			std::filesystem::path workingDirectory = std::filesystem::current_path();

			if (workingDirectory.stem().string() == "AntPlay")
				workingDirectory = workingDirectory.parent_path();

			Ant::FileSystem::SetEnvironmentVariable("ANT_DIR", workingDirectory.string());
		}

		PushLayer(new Ant::EditorLayer(m_UserPreferences));
	}
private:
	std::string m_ProjectPath;
	std::filesystem::path m_PersistentStoragePath;
	Ant::Ref<Ant::UserPreferences> m_UserPreferences;
};

Ant::Application* Ant::CreateApplication(int argc, char** argv)
{
	std::string_view projectPath;
	if (argc > 1)
		projectPath = argv[1];

	Ant::ApplicationSpecification specification;
	specification.Name = "AntPlay";
	specification.WindowWidth = 1600;
	specification.WindowHeight = 900;
	specification.StartMaximized = true;
	specification.VSync = true;
	//specification.RenderConfig.ShaderPackPath = "Resources/ShaderPack.asp";

	specification.ScriptConfig.CoreAssemblyPath = "Resources/Scripts/Ant-ScriptCore.dll";
	specification.ScriptConfig.EnableDebugging = true;
	specification.ScriptConfig.EnableProfiling = true;

	specification.CoreThreadingPolicy = ThreadingPolicy::SingleThreaded;

	return new AntPlayApplication(specification, projectPath);
}
