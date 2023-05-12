#pragma once
#include "Ant/Core/Timestep.h"
#include "Ant/Core/Events/ApplicationEvent.h"
#include "Ant/Core/Events/KeyEvent.h"
#include "Ant/Core/Events/MouseEvent.h"

namespace Ant{

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}
