#include <Ant.h>

class Sandbox : public Ant::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}


};

Ant::Application* Ant::CreateApplication()
{
	return new Sandbox();
}
