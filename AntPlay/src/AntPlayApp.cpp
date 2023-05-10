
#include <Ant.h>
#include "Ant/Core/EntryPoint.h"

#include "EditorLayer.h"

namespace Ant {
	class AntPlay : public Ant::Application
	{
	public:
		AntPlay(const Ant::ApplicationProps& props)
			: Application(props)
		{
			//PushLayer(new ExampleLayer);
			PushLayer(new EditorLayer());
			//PushLayer(new GameLayer());
		}

		virtual void OnInit() override
		{
			PushLayer(new Ant::EditorLayer());
		}
	};

	Ant::Application* Ant::CreateApplication()
	{
		return new AntPlay({ "AntPlay", 1600, 900 });
	}
}
