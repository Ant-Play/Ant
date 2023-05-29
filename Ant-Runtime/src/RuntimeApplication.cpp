#include <Ant/EntryPoint.h>
#include "RuntimeLayer.h"

class RuntimeApplication : public Ant::Application
{
public:
	RuntimeApplication(const Ant::ApplicationSpecification& specification, std::string_view projectPath)
		: Application(specification), m_ProjectPath(projectPath)
	{
		s_IsRuntime = true;
	}

	virtual void OnInit() override
	{
		PushLayer(new Ant::RuntimeLayer(m_ProjectPath));
	}
private:
	std::string m_ProjectPath;
};

Ant::Application* Ant::CreateApplication(int argc, char** argv)
{
	std::string_view projectPath = "SandboxProject/Sandbox.aproj";
	if (argc > 1)
		projectPath = argv[1];
	Ant::ApplicationSpecification specification;
	specification.Name = "Ant Runtime";
	specification.WindowWidth = 1280;
	specification.WindowHeight = 720;
	specification.WindowDecorated = true;
	specification.Fullscreen = false;
	specification.StartMaximized = false;
	specification.VSync = true;
	specification.EnableImGui = false;

	// IMPORTANT: Disable for ACTUAL Dist builds
	specification.WorkingDirectory = "../AntPlay";

	specification.ScriptConfig.CoreAssemblyPath = "Resources/Scripts/Ant-ScriptCore.dll";
	specification.ScriptConfig.EnableDebugging = false;
	specification.ScriptConfig.EnableProfiling = false;

	specification.RenderConfig.ShaderPackPath = "Resources/ShaderPack.asp";
	specification.RenderConfig.FramesInFlight = 3;

	specification.CoreThreadingPolicy = ThreadingPolicy::MultiThreaded;

	return new RuntimeApplication(specification, projectPath);
}