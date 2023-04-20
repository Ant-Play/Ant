#pragma once
#include "Ant/Core/Timestep.h"
#include "Ant/Events/ApplicationEvent.h"
#include "Ant/Events/KeyEvent.h"
#include "Ant/Events/MouseEvent.h"

namespace Ant{

	//��ʾͼ��Ļ������
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
