#pragma once

#include "Ant/Editor/EditorPanel.h"
#include "Ant/Renderer/SceneRenderer.h"

namespace Ant {

	class SceneRendererPanel : public EditorPanel
	{
	public:
		SceneRendererPanel() = default;
		virtual ~SceneRendererPanel() = default;

		void SetContext(const Ref<SceneRenderer>& context) { m_Context = context; }
		virtual void OnImGuiRender(bool& isOpen) override;
	private:
		Ref<SceneRenderer> m_Context;
	};

}
