#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

namespace Ant {

	class Input
	{
	public:

		static bool IsKeyPressed(KeyCode keycode);

		static bool IsMouseButtonPressed(MouseCode button);
		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePosition();

		static void SetCursorMode(CursorMode mode);
		static CursorMode GetCursorMode();
	};
}