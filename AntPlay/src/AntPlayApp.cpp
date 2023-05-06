
#include <Ant.h>
#include "Ant/Core/EntryPoint.h"

#include "EditorLayer.h"

namespace Ant {
	class AnePlay : public Ant::Application
	{
	public:
		AnePlay(ApplicationCommandLineArgs args)
			: Application("AntPlay", args)
		{
			//PushLayer(new ExampleLayer);
			PushLayer(new EditorLayer());
			//PushLayer(new GameLayer());
		}

		~AnePlay()
		{

		}


	};

	Ant::Application* Ant::CreateApplication(ApplicationCommandLineArgs args)
	{
		return new AnePlay(args);
	}
}
