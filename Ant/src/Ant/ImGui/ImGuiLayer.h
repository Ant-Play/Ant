#pragma once

#include "Ant/Layer.h"
#include "Ant/Events/ApplicationEvent.h"
#include "Ant/Events/KeyEvent.h"
#include "Ant/Events/MouseEvent.h"

namespace Ant {

	class ANT_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;

	};
}

