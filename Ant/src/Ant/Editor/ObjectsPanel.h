#pragma once

#include "Ant/Renderer/Texture.h"
#include "Ant/Asset/AssetManager.h"

namespace Ant{

	class ObjectsPanel
	{
	public:
		ObjectsPanel();

		void OnImGuiRender();

	private:
		void DrawObject(const char* label, AssetHandle handle);

	private:
		Ref<Texture2D> m_CubeImage;
	};
}
