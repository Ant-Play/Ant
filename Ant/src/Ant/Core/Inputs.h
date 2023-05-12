#pragma once

#include "Ant/Core/KeyCodes.h"
#include "Ant/Core/MouseCodes.h"

namespace Ant {

	class Input
	{
	public:

		static bool IsKeyPressed(KeyCode keycode);

		static bool IsMouseButtonPressed(MouseCode button);
		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePosition();
	};
}