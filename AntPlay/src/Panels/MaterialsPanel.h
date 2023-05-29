#pragma once

#include "Ant/Editor/EditorPanel.h"
#include "Ant/Scene/Entity.h"
#include "Ant/Renderer/Texture.h"

namespace Ant {

	class MaterialsPanel : public EditorPanel
	{
	public:
		MaterialsPanel();
		~MaterialsPanel();

		virtual void SetSceneContext(const Ref<Scene>& context) override;
		virtual void OnImGuiRender(bool& isOpen) override;

	private:
		void RenderMaterial(size_t materialIndex, AssetHandle materialAssetHandle);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectedEntity;
		Ref<Texture2D> m_CheckerBoardTexture;
	};

}
