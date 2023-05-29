#pragma once

#include "Ant/Scripts/ScriptEngine.h"

#include "EditorPanel.h"

namespace Ant {

	class ScriptEngineDebugPanel : public EditorPanel
	{
	public:
		ScriptEngineDebugPanel();
		~ScriptEngineDebugPanel();

		virtual void OnProjectChanged(const Ref<Project>& project) override;
		virtual void OnImGuiRender(bool& open) override;
	};

}
