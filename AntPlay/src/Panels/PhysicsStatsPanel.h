#pragma once

#include "Ant/Editor/EditorPanel.h"
#include "Ant/Physics/3D/PhysicsScene.h"

namespace Ant {

	class PhysicsStatsPanel : public EditorPanel
	{
	public:
		PhysicsStatsPanel();
		~PhysicsStatsPanel();

		virtual void SetSceneContext(const Ref<Scene>& context) override;
		virtual void OnImGuiRender(bool& isOpen) override;

	private:
		Ref<PhysicsScene> m_PhysicsScene;
	};

}
