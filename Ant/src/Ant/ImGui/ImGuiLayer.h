#pragma once

#include "Ant/Core/Layer.h"
#include "Ant/Core/Events/ApplicationEvent.h"
#include "Ant/Core/Events/KeyEvent.h"
#include "Ant/Core/Events/MouseEvent.h"

namespace Ant {

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		ImGuiLayer(const std::string& name);
		virtual ~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& event) override;
		virtual void OnImGuiRender() override;

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

