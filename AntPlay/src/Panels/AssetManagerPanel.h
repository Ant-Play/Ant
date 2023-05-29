#pragma once

#include "Ant/Editor/EditorPanel.h"

namespace Ant {

	class AssetManagerPanel : public EditorPanel
	{
	public:
		AssetManagerPanel() = default;
		virtual ~AssetManagerPanel() = default;

		virtual void OnImGuiRender(bool& isOpen) override;
	};

}

