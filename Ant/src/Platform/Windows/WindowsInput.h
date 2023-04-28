#pragma once
#include "Ant/Core/Inputs.h"

namespace Ant {

	class WindowsInputs : public Input
	{

	protected:
		virtual bool IsKeyPressedImpl(KeyCode key) override;

		virtual bool IsMouseButtonPressedImpl(MouseCode button) override;
		virtual std::pair<float, float> GetMousePositionImpl() override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	};
}
