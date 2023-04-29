
#include <Ant.h>
#include "Ant/Core/EntryPoint.h"
#include "Sandbox2D.h"

namespace Ant {

	class Sandbox : public Application
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

	Application* CreateApplication()
	{
		return new Sandbox();
	}
}
