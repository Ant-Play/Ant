#include "antpch.h"
#include "AssetEditorPanel.h"
#include "DefaultAssetEditors.h"
#include "Ant/Asset/AssetManager.h"

#include "Ant/Editor/NodeGraphEditor/SoundGraph/SoundGraphGraphEditor.h"

#include "MeshViewerPanel.h"
#include "MeshColliderEditor.h"

namespace Ant{

	AssetEditor::AssetEditor(const char* id)
		: m_Id(id), m_MinSize(200, 400), m_MaxSize(2000, 2000)
	{
		SetTitle(id);
	}

	void AssetEditor::OnImGuiRender()
	{
		if (!m_IsOpen)
			return;

		bool was_open = m_IsOpen;
		// NOTE: SetNextWindowSizeConstraints requires a max constraint that's above 0. For now we're just setting it to a large value
		OnWindowStylePush();
		ImGui::SetNextWindowSizeConstraints(m_MinSize, m_MaxSize);
		ImGui::Begin(m_TitleAndId.c_str(), &m_IsOpen, GetWindowFlags());
		OnWindowStylePop();
		Render();
		ImGui::End();

		if (was_open && !m_IsOpen)
			OnClose();
	}

	void AssetEditor::SetOpen(bool isOpen)
	{
		m_IsOpen = isOpen;
		if (!m_IsOpen)
			OnClose();
		else
			OnOpen();
	}

	void AssetEditor::SetMinSize(uint32_t width, uint32_t height)
	{
		if (width <= 0) width = 200;
		if (height <= 0) height = 400;

		m_MinSize = ImVec2(width, height);
	}

	void AssetEditor::SetMaxSize(uint32_t width, uint32_t height)
	{
		if (width <= 0) width = 2000;
		if (height <= 0) height = 2000;
		if (width <= m_MinSize.x) width = m_MinSize.x * 2;
		if (height <= m_MinSize.y) height = m_MinSize.y * 2;

		m_MaxSize = ImVec2(width, height);
	}

	void AssetEditor::SetTitle(const std::string& newTitle)
	{
		m_TitleAndId = newTitle + "###" + m_Id;
	}

	void AssetEditorPanel::RegisterDefaultEditors()
	{
		RegisterEditor<MaterialEditor>(AssetType::Material);
		RegisterEditor<TextureViewer>(AssetType::Texture);
		RegisterEditor<PhysicsMaterialEditor>(AssetType::PhysicsMat);
		RegisterEditor<AudioFileViewer>(AssetType::Audio);
		//RegisterEditor<MeshViewerPanel>(AssetType::MeshSource);
		RegisterEditor<PrefabEditor>(AssetType::Prefab);
		RegisterEditor<SoundConfigEditor>(AssetType::SoundConfig);
		RegisterEditor<MeshColliderEditor>(AssetType::MeshCollider);
		RegisterEditor<SoundGraphNodeGraphEditor>(AssetType::SoundGraphSound);
		RegisterEditor<AnimationControllerEditor>(AssetType::AnimationController);
	}

	void AssetEditorPanel::UnregisterAllEditors()
	{
		s_Editors.clear();
	}

	void AssetEditorPanel::OnUpdate(Timestep ts)
	{
		for (auto& kv : s_Editors)
			kv.second->OnUpdate(ts);
	}

	void AssetEditorPanel::OnEvent(Event& e)
	{
		for (auto& kv : s_Editors)
			kv.second->OnEvent(e);
	}

	void AssetEditorPanel::SetSceneContext(const Ref<Scene>& context)
	{
		s_SceneContext = context;

		for (auto& kv : s_Editors)
			kv.second->SetSceneContext(context);
	}

	void AssetEditorPanel::OnImGuiRender()
	{
		for (auto& kv : s_Editors)
			kv.second->OnImGuiRender();
	}

	void AssetEditorPanel::OpenEditor(const Ref<Asset>& asset)
	{
		if (asset == nullptr)
			return;

		if (s_Editors.find(asset->GetAssetType()) == s_Editors.end())
			return;

		auto& editor = s_Editors[asset->GetAssetType()];
		if (editor->IsOpen())
			editor->SetOpen(false);

		const auto& metadata = Project::GetEditorAssetManager()->GetMetadata(asset->Handle);
		editor->SetTitle(metadata.FilePath.filename().string());
		editor->SetAsset(AssetManager::GetAsset<Asset>(asset->Handle));
		editor->SetSceneContext(s_SceneContext);
		editor->SetOpen(true);
	}

	std::unordered_map<AssetType, Scope<AssetEditor>> AssetEditorPanel::s_Editors;
	Ref<Scene> AssetEditorPanel::s_SceneContext = nullptr;
}