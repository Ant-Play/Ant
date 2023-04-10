#pragma once
#include "Core.h"


namespace Ant{
	class ANT_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}

