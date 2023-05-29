#pragma once

#include "Ant/Core/Ref.h"
#include "Ant/Scene/Scene.h"
#include "Ant/Project/Project.h"
#include "Ant/Core/Events/Event.h"

namespace Ant {

	class EditorPanel : public RefCounted
	{
	public:
		virtual ~EditorPanel() = default;

		virtual void OnImGuiRender(bool& isOpen) = 0;
		virtual void OnEvent(Event& e) {}
		virtual void OnProjectChanged(const Ref<Project>& project) {}
		virtual void SetSceneContext(const Ref<Scene>& context) {}
	};

}