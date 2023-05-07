#pragma once

#include "Ant/Core/Base.h"
#include "Ant/Core/KeyCodes.h"
#include "Ant/Core/MouseCodes.h"

namespace Ant {
	// ‰»ÎIOµƒ≥ÈœÛ
	class Input
	{
	public:

		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		static void SetCursorMode(CursorMode mode);
	};
}