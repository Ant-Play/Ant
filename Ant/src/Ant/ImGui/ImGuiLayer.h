#pragma once

#include "Ant/Core/Layer.h"
#include "Ant/Events/ApplicationEvent.h"
#include "Ant/Events/KeyEvent.h"
#include "Ant/Events/MouseEvent.h"

namespace Ant {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& event) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;
	private:
		float m_Time = 0.0f;
		bool m_BlockEvents = true;
	};
}

