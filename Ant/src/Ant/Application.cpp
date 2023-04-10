#include "antpch.h"
#include "Application.h"

#include "Ant/Events/ApplicationEvent.h"
#include "Ant/Log.h"

namespace Ant {

	Application::Application()
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{
		WindowsResizeEvent e(1280, 720);
		if (e.IsInCategory(EventCategoryApplication))
		{
			ANT_TRACE(e);
		}
		if (!e.IsInCategory(EventCategoryInput))
		{
			ANT_ERROR(e);
		}
		while (true);
	}
}
