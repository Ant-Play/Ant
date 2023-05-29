#pragma once

#include "Ant/Core/Layer.h"

namespace Ant {

	class ImGuiLayer : public Layer
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;

		void SetDarkThemeColors();
		void SetDarkThemeV2Colors();

		void AllowInputEvents(bool allowEvents);

		static ImGuiLayer* Create();
	};
}

