#pragma once

#include "AssetEditorPanel.h"
#include "Ant/Renderer/Mesh.h"

#include "Ant/Scene/Prefab.h"
#include "Ant/Editor/SceneHierarchyPanel.h"

namespace Audio
{
	struct SoundConfig;
}

namespace Ant{

	class AnimationController;
	struct SoundConfig;

	class PhysicsMaterialEditor : public AssetEditor
	{
	public:
		PhysicsMaterialEditor();

		virtual void SetAsset(const Ref<Asset>& asset) override { m_Asset = (Ref<PhysicsMaterial>)asset; }

	private:
		virtual void OnOpen() override;
		virtual void OnClose() override;
		virtual void Render() override;

	private:
		Ref<PhysicsMaterial> m_Asset;
	};

	class MaterialEditor : public AssetEditor
	{
	public:
		MaterialEditor();

		virtual void SetAsset(const Ref<Asset>& asset) override { m_MaterialAsset = (Ref<MaterialAsset>)asset; }

	private:
		virtual void OnOpen() override;
		virtual void OnClose() override;
		virtual void Render() override;

	private:
		Ref<MaterialAsset> m_MaterialAsset;
	};

	class PrefabEditor : public AssetEditor
	{
	public:
		PrefabEditor();

		virtual void SetAsset(const Ref<Asset>& asset) override { m_Prefab = (Ref<Prefab>)asset; m_SceneHierarchyPanel.SetSceneContext(m_Prefab->m_Scene); }

	private:
		virtual void OnOpen() override;
		virtual void OnClose() override;
		virtual void Render() override;

	private:
		Ref<Prefab> m_Prefab;
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};

	class TextureViewer : public AssetEditor
	{
	public:
		TextureViewer();

		virtual void SetAsset(const Ref<Asset>& asset) override { m_Asset = (Ref<Texture>)asset; }

	private:
		virtual void OnOpen() override;
		virtual void OnClose() override;
		virtual void Render() override;

	private:
		Ref<Texture> m_Asset;
	};

	class AudioFileViewer : public AssetEditor
	{
	public:
		AudioFileViewer();

		virtual void SetAsset(const Ref<Asset>& asset) override;

	private:
		virtual void OnOpen() override;
		virtual void OnClose() override;
		virtual void Render() override;

	private:
		Ref<AudioFile> m_Asset;
	};

	class SoundConfigEditor : public AssetEditor
	{
	public:
		SoundConfigEditor();

		virtual void SetAsset(const Ref<Asset>& asset) override;

	private:
		virtual void OnOpen() override;
		virtual void OnClose() override;
		virtual void Render() override;

	private:
		Ref<SoundConfig> m_Asset;
	};

	class AnimationControllerEditor : public AssetEditor
	{
	public:
		AnimationControllerEditor();

		virtual void SetAsset(const Ref<Asset>& asset) override;

	private:
		virtual void OnOpen() override;
		virtual void OnClose() override;
		virtual void Render() override;

		void UI_CreateNewSkeletonPopup();
		void UI_CreateNewAnimationPopup();
		void UI_NoSkeletonPopup();
		void UI_NoAnimationPopup();
		void UI_InvalidMetadataPopup();

	private:
		inline static float FirstColumnWidth = 200.0f;

		struct CreateNewAssetPopupData
		{
			Ref<MeshSource> MeshSource = nullptr;
			std::array<char, 256> CreateAssetFilenameBuffer = {};
			AssetHandle SkeletonHandle = 0;
			AssetHandle AnimationHandle = 0;
			bool GotSkeleton = false;
			bool GotAnimation = false;
			uint32_t State = ~0;

			void Reset()
			{
				MeshSource = nullptr;
				CreateAssetFilenameBuffer.fill(0);
			}

		} m_CreateNewAssetPopupData;

		AssetMetadata m_InvalidMetadata;

		Ref<AnimationController> m_Asset;
		std::vector<float> m_GridColumn0Widths;
		bool m_ShowCreateNewSkeletonPopup = false;
		bool m_ShowCreateNewAnimationPopup = false;
		bool m_ShowNoSkeletonPopup = false;
		bool m_ShowNoAnimationPopup = false;
		bool m_ShowInvalidMetadataPopup = false;
	};
}
