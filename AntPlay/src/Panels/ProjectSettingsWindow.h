#pragma once

#include "Ant/Core/Log.h"
#include "Ant/Project/Project.h"
#include "Ant/Scene/Scene.h"

#include "Ant/Editor/EditorPanel.h"

namespace Ant {

	class ProjectSettingsWindow : public EditorPanel
	{
	public:
		ProjectSettingsWindow();
		~ProjectSettingsWindow();

		virtual void OnImGuiRender(bool& isOpen) override;
		virtual void OnProjectChanged(const Ref<Project>& project) override;

	private:
		void RenderGeneralSettings();
		void RenderRendererSettings();
		void RenderAudioSettings();
		void RenderScriptingSettings();
		void RenderPhysicsSettings();
		void RenderLogSettings();
	private:
		Ref<Project> m_Project;
		AssetHandle m_DefaultScene;
		int32_t m_SelectedLayer = -1;
		char m_NewLayerNameBuffer[255];

	};

}
