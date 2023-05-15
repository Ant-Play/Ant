
#include <Ant.h>
#include <Ant/EntryPoint.h>

#include "EditorLayer.h"

class AntPlayApplication : public Ant::Application
{
public:
	AntPlayApplication(const Ant::ApplicationProps& props)
		: Application(props)
	{
	}

	virtual void OnInit() override
	{
		PushLayer(new Ant::EditorLayer());
	}
};

Ant::Application* Ant::CreateApplication(int argc, char** argv)
{
	return new AntPlayApplication({ "AntPlay", 1600, 900 });
}
