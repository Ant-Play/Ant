
#include <Ant.h>
#include "Ant/Core/EntryPoint.h"

#include "EditorLayer.h"

namespace Ant {
	class AnePlay : public Ant::Application
	{
	public:
		AnePlay()
			: Application("AntPlay")
		{
			//PushLayer(new ExampleLayer);
			PushLayer(new EditorLayer());
			//PushLayer(new GameLayer());
		}

		~AnePlay()
		{

		}


	};

	Ant::Application* Ant::CreateApplication()
	{
		return new AnePlay();
	}
}
