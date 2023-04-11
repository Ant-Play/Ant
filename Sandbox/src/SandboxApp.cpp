#include <Ant.h>
#include <iostream>
class ExampleLayer : public Ant::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{

	}

	void OnUpdate()
	{
		ANT_INFO("ExampleLayer::Update");
	}

	void OnEvent(Ant::Event& event)
	{
		//ANT_TRACE("{0}", event);
		ANT_TRACE("Event has running");
	}
};

class Sandbox : public Ant::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer);
		PushOverlay(new Ant::ImGuiLayer());
	}

	~Sandbox()
	{

	}


};

Ant::Application* Ant::CreateApplication()
{
	return new Sandbox();
}
