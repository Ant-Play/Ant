
#include <Ant.h>
#include "Ant/Core/EntryPoint.h"
#include "Sandbox2D.h"

namespace Ant {

	class Sandbox : public Application
	{
	public:
		Sandbox(const Ant::ApplicationSpecification& specification)
			: Application(specification)
		{
			//PushLayer(new ExampleLayer);
			PushLayer(new Sandbox2D());
			//PushLayer(new GameLayer());
		}

		~Sandbox()
		{

		}


	};

	Application* CreateApplication(Ant::ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Sandbox";
		spec.WorkingDirectory = "../Hazelnut";
		spec.CommandLineArgs = args;

		return new Sandbox(spec);
	}
}
