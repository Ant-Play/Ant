
#include <Ant.h>
#include "Ant/Core/EntryPoint.h"

#include "EditorLayer.h"

namespace Ant {
	class AntPlay : public Ant::Application
	{
	public:
		AntPlay(const ApplicationSpecification& specification)
			: Application(specification)
		{
			//PushLayer(new ExampleLayer);
			PushLayer(new EditorLayer());
			//PushLayer(new GameLayer());
		}

	};

	Ant::Application* Ant::CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "AntPlay";
		spec.CommandLineArgs = args;

		return new AntPlay(spec);
	}
}
