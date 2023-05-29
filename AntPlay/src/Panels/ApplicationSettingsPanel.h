#pragma once

#include "Ant/Editor/EditorPanel.h"
#include "Ant/Renderer/Texture.h"

#include <functional>

namespace Ant {

	struct SettingsPage
	{
		using PageRenderFunction = std::function<void()>;

		const char* Name;
		PageRenderFunction RenderFunction;
	};

	class ApplicationSettingsPanel : public EditorPanel
	{
	public:
		ApplicationSettingsPanel();
		~ApplicationSettingsPanel();

		virtual void OnImGuiRender(bool& isOpen) override;

	private:
		void DrawPageList();

		void DrawRendererPage();
		void DrawScriptingPage();
		void DrawAntPlayPage();
	private:
		uint32_t m_CurrentPage = 0;
		std::vector<SettingsPage> m_Pages;
	};

}