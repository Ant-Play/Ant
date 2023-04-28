
#include <Ant.h>
#include "Ant/Core/EntryPoint.h"
#include "Sandbox2D.h"

class Sandbox : public Ant::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer);
		PushLayer(new Sandbox2D());
		//PushLayer(new GameLayer());
	}

	~Sandbox()
	{

	}


};

Ant::Application* Ant::CreateApplication()
{
	return new Sandbox();
}
