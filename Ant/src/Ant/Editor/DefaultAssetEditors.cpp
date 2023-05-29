#include "antpch.h"
#include "DefaultAssetEditors.h"
#include "Ant/Asset/AssetImporter.h"
#include "Ant/Asset/AssetManager.h"
#include "Ant/Audio/AudioFileUtils.h"
#include "Ant/Audio/Sound.h"
#include "Ant/Editor/NodeGraphEditor/NodeGraphAsset.h"
#include "Ant/Renderer/Renderer.h"
#include "Ant/Editor/SelectionManager.h"
#include "Ant/Audio/AudioEngine.h"

#include "imgui_internal.h"

namespace Ant{

	PhysicsMaterialEditor::PhysicsMaterialEditor()
		: AssetEditor("Edit Physics Material") {}

	void PhysicsMaterialEditor::OnOpen()
	{
		if (!m_Asset)
			SetOpen(false);
	}

	void PhysicsMaterialEditor::OnClose()
	{
		if (m_Asset)
			AssetImporter::Serialize(Project::GetEditorAssetManager()->GetMetadata(m_Asset->Handle), m_Asset);

		m_Asset = nullptr;
	}

	void PhysicsMaterialEditor::Render()
	{

		UI::BeginPropertyGrid();
		UI::Property("Static Friction", m_Asset->StaticFriction);
		UI::Property("Dynamic Friction", m_Asset->DynamicFriction);
		UI::Property("Bounciness", m_Asset->Bounciness);
		UI::EndPropertyGrid();
	}

	MaterialEditor::MaterialEditor()
		: AssetEditor("Material Editor")
	{
	}

	void MaterialEditor::OnOpen()
	{
		if (!m_MaterialAsset)
			SetOpen(false);
	}

	void MaterialEditor::OnClose()
	{
		m_MaterialAsset = nullptr;
	}

	void MaterialEditor::Render()
	{
		auto material = m_MaterialAsset->GetMaterial();

		Ref<Shader> shader = material->GetShader();
		bool needsSerialize = false;

		Ref<Shader> transparentShader = Renderer::GetShaderLibrary()->Get("AntPBR_Transparent");
		bool transparent = shader == transparentShader;
		UI::BeginPropertyGrid();
		UI::PushID();
		if (UI::Property("Transparent", transparent))
		{
			if (transparent)
				m_MaterialAsset->SetMaterial(Material::Create(transparentShader));
			else
				m_MaterialAsset->SetMaterial(Material::Create(Renderer::GetShaderLibrary()->Get("AntPBR_Static")));

			m_MaterialAsset->m_Transparent = transparent;
			m_MaterialAsset->SetDefaults();

			material = m_MaterialAsset->GetMaterial();
			shader = material->GetShader();
		}
		UI::PopID();
		UI::EndPropertyGrid();

		ImGui::Text("Shader: %s", material->GetShader()->GetName().c_str());

		// Albedo
		if (ImGui::CollapsingHeader("Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));

			auto& albedoColor = material->GetVector3("u_MaterialUniforms.AlbedoColor");
			Ref<Texture2D> albedoMap = material->TryGetTexture2D("u_AlbedoTexture");
			bool hasAlbedoMap = albedoMap ? !albedoMap.EqualsObject(Renderer::GetWhiteTexture()) && albedoMap->GetImage() : false;
			Ref<Texture2D> albedoUITexture = hasAlbedoMap ? albedoMap : EditorResources::CheckerboardTexture;

			ImVec2 textureCursorPos = ImGui::GetCursorPos();
			UI::Image(albedoUITexture, ImVec2(64, 64));
			if (ImGui::BeginDragDropTarget())
			{
				auto data = ImGui::AcceptDragDropPayload("asset_payload");
				if (data)
				{
					int count = data->DataSize / sizeof(AssetHandle);

					for (int i = 0; i < count; i++)
					{
						if (count > 1)
							break;

						AssetHandle assetHandle = *(((AssetHandle*)data->Data) + i);
						Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
						if (!asset || asset->GetAssetType() != AssetType::Texture)
							break;

						albedoMap = asset.As<Texture2D>();
						material->Set("u_AlbedoTexture", albedoMap);
						needsSerialize = true;
					}
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::PopStyleVar();
			if (ImGui::IsItemHovered())
			{
				if (hasAlbedoMap)
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					std::string filepath = albedoMap->GetPath().string();
					ImGui::TextUnformatted(filepath.c_str());
					ImGui::PopTextWrapPos();
					UI::Image(albedoUITexture, ImVec2(384, 384));
					ImGui::EndTooltip();
				}
				if (ImGui::IsItemClicked())
				{
				}
			}

			ImVec2 nextRowCursorPos = ImGui::GetCursorPos();
			ImGui::SameLine();
			ImVec2 properCursorPos = ImGui::GetCursorPos();
			ImGui::SetCursorPos(textureCursorPos);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			if (hasAlbedoMap && ImGui::Button("X##AlbedoMap", ImVec2(18, 18)))
			{
				m_MaterialAsset->ClearAlbedoMap();
				needsSerialize = true;
			}
			ImGui::PopStyleVar();
			ImGui::SetCursorPos(properCursorPos);

			ImGui::ColorEdit3("Color##Albedo", glm::value_ptr(albedoColor), ImGuiColorEditFlags_NoInputs);
			if (ImGui::IsItemDeactivated())
				needsSerialize = true;

			float& emissive = material->GetFloat("u_MaterialUniforms.Emission");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.0f);
			ImGui::DragFloat("Emission", &emissive, 0.1f, 0.0f, 20.0f);
			if (ImGui::IsItemDeactivated())
				needsSerialize = true;

			ImGui::SetCursorPos(nextRowCursorPos);
		}

		if (transparent)
		{
			float& transparency = material->GetFloat("u_MaterialUniforms.Transparency");

			UI::BeginPropertyGrid();
			UI::Property("Transparency", transparency, 0.01f, 0.0f, 1.0f);
			UI::EndPropertyGrid();
		}
		else
		{
			// Textures ------------------------------------------------------------------------------
			{
				// Normals
				if (ImGui::CollapsingHeader("Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
					bool useNormalMap = material->GetFloat("u_MaterialUniforms.UseNormalMap");
					Ref<Texture2D> normalMap = material->TryGetTexture2D("u_NormalTexture");

					bool hasNormalMap = normalMap ? !normalMap.EqualsObject(Renderer::GetWhiteTexture()) && normalMap->GetImage() : false;
					ImVec2 textureCursorPos = ImGui::GetCursorPos();
					UI::Image(hasNormalMap ? normalMap : EditorResources::CheckerboardTexture, ImVec2(64, 64));

					if (ImGui::BeginDragDropTarget())
					{
						auto data = ImGui::AcceptDragDropPayload("asset_payload");
						if (data)
						{
							int count = data->DataSize / sizeof(AssetHandle);

							for (int i = 0; i < count; i++)
							{
								if (count > 1)
									break;

								AssetHandle assetHandle = *(((AssetHandle*)data->Data) + i);
								Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
								if (!asset || asset->GetAssetType() != AssetType::Texture)
									break;

								normalMap = asset.As<Texture2D>();
								material->Set("u_NormalTexture", normalMap);
								material->Set("u_MaterialUniforms.UseNormalMap", true);
								needsSerialize = true;
							}
						}

						ImGui::EndDragDropTarget();
					}

					ImGui::PopStyleVar();
					if (ImGui::IsItemHovered())
					{
						if (hasNormalMap)
						{
							ImGui::BeginTooltip();
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							std::string filepath = normalMap->GetPath().string();
							ImGui::TextUnformatted(filepath.c_str());
							ImGui::PopTextWrapPos();
							UI::Image(normalMap, ImVec2(384, 384));
							ImGui::EndTooltip();
						}
						if (ImGui::IsItemClicked())
						{
						}
					}
					ImVec2 nextRowCursorPos = ImGui::GetCursorPos();
					ImGui::SameLine();
					ImVec2 properCursorPos = ImGui::GetCursorPos();
					ImGui::SetCursorPos(textureCursorPos);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					if (hasNormalMap && ImGui::Button("X##NormalMap", ImVec2(18, 18)))
					{
						m_MaterialAsset->ClearNormalMap();
						needsSerialize = true;
					}
					ImGui::PopStyleVar();
					ImGui::SetCursorPos(properCursorPos);

					if (ImGui::Checkbox("Use##NormalMap", &useNormalMap))
						material->Set("u_MaterialUniforms.UseNormalMap", useNormalMap);

					if (ImGui::IsItemDeactivated())
						needsSerialize = true;

					ImGui::SetCursorPos(nextRowCursorPos);
				}
			}
			{
				// Metalness
				if (ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
					float& metalnessValue = material->GetFloat("u_MaterialUniforms.Metalness");
					Ref<Texture2D> metalnessMap = material->TryGetTexture2D("u_MetalnessTexture");

					bool hasMetalnessMap = metalnessMap ? !metalnessMap.EqualsObject(Renderer::GetWhiteTexture()) && metalnessMap->GetImage() : false;
					ImVec2 textureCursorPos = ImGui::GetCursorPos();
					UI::Image(hasMetalnessMap ? metalnessMap : EditorResources::CheckerboardTexture, ImVec2(64, 64));

					if (ImGui::BeginDragDropTarget())
					{
						auto data = ImGui::AcceptDragDropPayload("asset_payload");
						if (data)
						{
							int count = data->DataSize / sizeof(AssetHandle);

							for (int i = 0; i < count; i++)
							{
								if (count > 1)
									break;

								AssetHandle assetHandle = *(((AssetHandle*)data->Data) + i);
								Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
								if (!asset || asset->GetAssetType() != AssetType::Texture)
									break;

								metalnessMap = asset.As<Texture2D>();
								material->Set("u_MetalnessTexture", metalnessMap);
								needsSerialize = true;
							}
						}

						ImGui::EndDragDropTarget();
					}

					ImGui::PopStyleVar();
					if (ImGui::IsItemHovered())
					{
						if (hasMetalnessMap)
						{
							ImGui::BeginTooltip();
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							std::string filepath = metalnessMap->GetPath().string();
							ImGui::TextUnformatted(filepath.c_str());
							ImGui::PopTextWrapPos();
							UI::Image(metalnessMap, ImVec2(384, 384));
							ImGui::EndTooltip();
						}
						if (ImGui::IsItemClicked())
						{
						}
					}
					ImVec2 nextRowCursorPos = ImGui::GetCursorPos();
					ImGui::SameLine();
					ImVec2 properCursorPos = ImGui::GetCursorPos();
					ImGui::SetCursorPos(textureCursorPos);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					if (hasMetalnessMap && ImGui::Button("X##MetalnessMap", ImVec2(18, 18)))
					{
						m_MaterialAsset->ClearMetalnessMap();
						needsSerialize = true;
					}
					ImGui::PopStyleVar();
					ImGui::SetCursorPos(properCursorPos);
					ImGui::SetNextItemWidth(200.0f);
					ImGui::SliderFloat("Metalness Value##MetalnessInput", &metalnessValue, 0.0f, 1.0f);
					if (ImGui::IsItemDeactivated())
						needsSerialize = true;
					ImGui::SetCursorPos(nextRowCursorPos);
				}
			}
			{
				// Roughness
				if (ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
					float& roughnessValue = material->GetFloat("u_MaterialUniforms.Roughness");
					Ref<Texture2D> roughnessMap = material->TryGetTexture2D("u_RoughnessTexture");
					bool hasRoughnessMap = roughnessMap ? !roughnessMap.EqualsObject(Renderer::GetWhiteTexture()) && roughnessMap->GetImage() : false;
					ImVec2 textureCursorPos = ImGui::GetCursorPos();
					UI::Image(hasRoughnessMap ? roughnessMap : EditorResources::CheckerboardTexture, ImVec2(64, 64));

					if (ImGui::BeginDragDropTarget())
					{
						auto data = ImGui::AcceptDragDropPayload("asset_payload");
						if (data)
						{
							int count = data->DataSize / sizeof(AssetHandle);

							for (int i = 0; i < count; i++)
							{
								if (count > 1)
									break;

								AssetHandle assetHandle = *(((AssetHandle*)data->Data) + i);
								Ref<Asset> asset = AssetManager::GetAsset<Asset>(assetHandle);
								if (!asset || asset->GetAssetType() != AssetType::Texture)
									break;

								roughnessMap = asset.As<Texture2D>();
								material->Set("u_RoughnessTexture", roughnessMap);
								needsSerialize = true;
							}
						}

						ImGui::EndDragDropTarget();
					}

					ImGui::PopStyleVar();
					if (ImGui::IsItemHovered())
					{
						if (hasRoughnessMap)
						{
							ImGui::BeginTooltip();
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							std::string filepath = roughnessMap->GetPath().string();
							ImGui::TextUnformatted(filepath.c_str());
							ImGui::PopTextWrapPos();
							UI::Image(roughnessMap, ImVec2(384, 384));
							ImGui::EndTooltip();
						}
						if (ImGui::IsItemClicked())
						{

						}
					}
					ImVec2 nextRowCursorPos = ImGui::GetCursorPos();
					ImGui::SameLine();
					ImVec2 properCursorPos = ImGui::GetCursorPos();
					ImGui::SetCursorPos(textureCursorPos);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					if (hasRoughnessMap && ImGui::Button("X##RoughnessMap", ImVec2(18, 18)))
					{
						m_MaterialAsset->ClearRoughnessMap();
						needsSerialize = true;
					}
					ImGui::PopStyleVar();
					ImGui::SetCursorPos(properCursorPos);
					ImGui::SetNextItemWidth(200.0f);
					ImGui::SliderFloat("Roughness Value##RoughnessInput", &roughnessValue, 0.0f, 1.0f);
					if (ImGui::IsItemDeactivated())
						needsSerialize = true;
					ImGui::SetCursorPos(nextRowCursorPos);
				}
			}

			UI::BeginPropertyGrid();

			bool castsShadows = m_MaterialAsset->IsShadowCasting();
			if (UI::Property("Casts shadows", castsShadows))
				m_MaterialAsset->SetShadowCasting(castsShadows);

			UI::EndPropertyGrid();
		}

		if (needsSerialize)
			AssetImporter::Serialize(m_MaterialAsset);
	}

	TextureViewer::TextureViewer()
		: AssetEditor("Edit Texture")
	{
		SetMinSize(200, 600);
		SetMaxSize(500, 1000);
	}

	void TextureViewer::OnOpen()
	{
		if (!m_Asset)
			SetOpen(false);
	}

	void TextureViewer::OnClose()
	{
		m_Asset = nullptr;
	}

	void TextureViewer::Render()
	{
		float textureWidth = (float)m_Asset->GetWidth();
		float textureHeight = (float)m_Asset->GetHeight();
		//float bitsPerPixel = Texture::GetBPP(m_Asset->GetFormat());
		float imageSize = ImGui::GetWindowWidth() - 40;
		imageSize = glm::min(imageSize, 500.0f);

		ImGui::SetCursorPosX(20);
		//ImGui::Image((ImTextureID)m_Asset->GetRendererID(), { imageSize, imageSize });

		UI::BeginPropertyGrid();
		UI::BeginDisabled();
		UI::Property("Width", textureWidth);
		UI::Property("Height", textureHeight);
		// UI::Property("Bits", bitsPerPixel, 0.1f, 0.0f, 0.0f, true); // TODO: Format
		UI::EndDisabled();
		UI::EndPropertyGrid();
	}

	AudioFileViewer::AudioFileViewer()
		: AssetEditor("Audio File")
	{
		SetMinSize(340, 340);
		SetMaxSize(500, 500);
	}

	void AudioFileViewer::SetAsset(const Ref<Asset>& asset)
	{
		m_Asset = (Ref<AudioFile>)asset;

		const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(m_Asset.As<AudioFile>()->Handle);
		SetTitle(metadata.FilePath.stem().string());
	}

	void AudioFileViewer::OnOpen()
	{
		if (!m_Asset)
			SetOpen(false);
	}

	void AudioFileViewer::OnClose()
	{
		m_Asset = nullptr;
	}

	void AudioFileViewer::Render()
	{
		std::chrono::duration<double> seconds{ m_Asset->Duration };
		auto duration = Utils::DurationToString(seconds);

		auto samplingRate = std::to_string(m_Asset->SamplingRate) + " Hz";
		auto bitDepth = std::to_string(m_Asset->BitDepth) + "-bit";
		auto channels = AudioFileUtils::ChannelsToLayoutString(m_Asset->NumChannels);
		auto fileSize = Utils::BytesToString(m_Asset->FileSize);
		auto& filePath = Project::GetEditorAssetManager()->GetMetadata(m_Asset->Handle).FilePath.string();

		auto localBounds = ImGui::GetContentRegionAvail();
		localBounds.y -= 14.0f; // making sure to not overlap resize corner

		// Hacky way to hide Property widget background, while making overall text contrast more pleasant
		auto& colors = ImGui::GetStyle().Colors;
		ImGui::PushStyleColor(ImGuiCol_ChildBg, colors[ImGuiCol_FrameBg]);

		ImGui::BeginChild("AudioFileProps", localBounds, false);
		ImGui::Spacing();

		UI::BeginPropertyGrid();
		UI::Property("Duration", duration.c_str());
		UI::Property("Sampling rate", samplingRate.c_str());
		UI::Property("Bit depth", bitDepth.c_str());
		UI::Property("Channels", channels.c_str());
		UI::Property("File size", fileSize.c_str());
		UI::Property("File path", filePath.c_str());
		UI::EndPropertyGrid();

		ImGui::EndChild();

		ImGui::PopStyleColor(); // ImGuiCol_ChildBg
	}

	SoundConfigEditor::SoundConfigEditor()
		: AssetEditor("Sound Configuration")
	{
		SetMinSize(340, 340);
		SetMaxSize(700, 700);
	}

	void SoundConfigEditor::SetAsset(const Ref<Asset>& asset)
	{
		MiniAudioEngine::Get().StopActiveAudioFilePreview();

		m_Asset = (Ref<SoundConfig>)asset;

		const AssetMetadata& metadata = Project::GetEditorAssetManager()->GetMetadata(m_Asset.As<SoundConfig>()->Handle);
		SetTitle(metadata.FilePath.stem().string());
	}

	void SoundConfigEditor::OnOpen()
	{
		MiniAudioEngine::Get().StopActiveAudioFilePreview();

		if (!m_Asset)
			SetOpen(false);
	}

	void SoundConfigEditor::OnClose()
	{
		MiniAudioEngine::Get().StopActiveAudioFilePreview();

		if (m_Asset)
			AssetImporter::Serialize(m_Asset);

		m_Asset = nullptr;
	}

	void SoundConfigEditor::Render()
	{
		auto localBounds = ImGui::GetContentRegionAvail();
		localBounds.y -= 14.0f; // making sure to not overlap resize corner

		ImGui::BeginChild("SoundConfigPanel", localBounds, false);
		ImGui::Spacing();

		{
			// PropertyGrid consists out of 2 columns, so need to move cursor accordingly
			auto propertyGridSpacing = []
			{
				ImGui::Spacing();
				ImGui::NextColumn();
				ImGui::NextColumn();
			};

			//=======================================================

			SoundConfig& soundConfig = *m_Asset;

			// Adding space after header
			ImGui::Spacing();

			//--- Sound Assets and Looping
			//----------------------------
			UI::PushID();
			UI::BeginPropertyGrid();
			// Need to wrap this first Property Grid into another ID,
			// otherwise there's a conflict with the next Property Grid.

			{
				UI::PropertyAssetReferenceError error;
				UI::PropertyAssetReferenceSettings settings;
				settings.AllowMemoryOnlyAssets = false;
				settings.ShowFullFilePath = false;
				UI::PropertyMultiAssetReference<AssetType::Audio, AssetType::SoundGraphSound>("Source", soundConfig.DataSourceAsset, &error, settings);
			}

			if (soundConfig.DataSourceAsset)
			{
				const auto path = Project::GetEditorAssetManager()->GetFileSystemPath(soundConfig.DataSourceAsset);
				if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path))
				{
					const bool isValidAudioFile = AudioFileUtils::IsValidAudioFile(path.string());

					UI::BeginDisabled(!isValidAudioFile);

					ImGui::NextColumn();

					if (ImGui::Button("Preview"))
					{
						MiniAudioEngine::Get().PreviewAudioFile(soundConfig.DataSourceAsset);
					}

					ImGui::SameLine();

					if (ImGui::Button("Stop"))
					{
						MiniAudioEngine::Get().StopActiveAudioFilePreview();
					}

					ImGui::NextColumn();

					UI::EndDisabled();
				}
			}

			propertyGridSpacing();

			UI::Property("Volume Multiplier", soundConfig.VolumeMultiplier, 0.01f, 0.0f, 1.0f); //TODO: switch to dBs in the future ?
			UI::Property("Pitch Multiplier", soundConfig.PitchMultiplier, 0.01f, 0.0f, 24.0f); // max pitch 24 is just an arbitrary number here

			propertyGridSpacing();
			propertyGridSpacing();

			UI::Property("Looping", soundConfig.bLooping);

			propertyGridSpacing();
			propertyGridSpacing();

			// Currently we use normalized value for the filter properties,
			// which are then translated to frequency scale

			//? was trying to make a more usable logarithmic scale for the slider value,
			//? so that 0.5 represents 1.2 kHz, or 0.0625 of frequency range normalized
			auto logFrequencyToNormScale = [](float frequencyN)
			{
				return (log2(frequencyN) + 8.0f) / 8.0f;
			};
			auto sliderScaleToLogFreq = [](float sliderValue)
			{
				return pow(2.0f, 8.0f * sliderValue - 8.0f);
			};

			//float lpfFreq = logFrequencyToNormScale(1.0f - soundConfig.LPFilterValue * 0.99609375f);
			float lpfFreq = /*1.0f -*/ soundConfig.LPFilterValue;
			if (UI::Property("Low-Pass Filter", lpfFreq, 0.0f, 0.0f, 1.0f))
			{
				lpfFreq = std::clamp(lpfFreq, 0.0f, 1.0f);
				//soundConfig.LPFilterValue = sliderScaleToLogFreq(lpfFreq);
				soundConfig.LPFilterValue = /*1.0f - */lpfFreq;
			}

			//float hpfFreq = logFrequencyToNormScale(soundConfig.HPFilterValue * 0.99609375f + 0.00390625f);
			float hpfFreq = soundConfig.HPFilterValue;
			if (UI::Property("High-Pass Filter", hpfFreq, 0.0f, 0.0f, 1.0f))
			{
				hpfFreq = std::clamp(hpfFreq, 0.0f, 1.0f);
				//soundConfig.HPFilterValue = sliderScaleToLogFreq(1.0f - hpfFreq);
				soundConfig.HPFilterValue = hpfFreq;
			}

			propertyGridSpacing();
			propertyGridSpacing();

			UI::Property("Master Reverb send", soundConfig.MasterReverbSend, 0.01f, 0.0f, 1.0f);

			UI::EndPropertyGrid();
			UI::PopID();

			ImGui::Spacing();
			ImGui::Spacing();

			auto contentRegionAvailable = ImGui::GetContentRegionAvail();

			//--- Enable Spatialization
			//-------------------------
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 6.0f });
			bool spatializationOpen = UI::PropertyGridHeader("Spatialization", true);
			ImGui::PopStyleVar(); // ImGuiStyleVar_FramePadding
			ImGui::SameLine(contentRegionAvailable.x - (ImGui::GetFrameHeight() + GImGui->Style.FramePadding.y * 2.0f));
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f);
			ImGui::Checkbox("##enabled", &soundConfig.bSpatializationEnabled);
			UI::DrawItemActivityOutline(GImGui->Style.FrameRounding, true, Colours::Theme::accent);

			ImGui::Indent(12.0f);

			//--- Spatialization Settings
			//---------------------------
			if (spatializationOpen)
			{
				ImGui::Spacing();

				using AttModel = AttenuationModel;

				auto& spatialConfig = soundConfig.Spatialization;

				auto getTextForModel = [&](AttModel model)
				{
					switch (model)
					{
					case AttModel::None:
						return "None";
					case AttModel::Inverse:
						return "Inverse";
					case AttModel::Linear:
						return "Linear";
					case AttModel::Exponential:
						return "Exponential";
					default:
						return "None";
					}
				};

				const auto& attenModStr = std::vector<std::string>{ getTextForModel(AttModel::None),
																	getTextForModel(AttModel::Inverse),
																	getTextForModel(AttModel::Linear),
																	getTextForModel(AttModel::Exponential) };

				UI::BeginPropertyGrid();

				int32_t selectedModel = static_cast<int32_t>(spatialConfig->AttenuationMod);
				if (UI::PropertyDropdown("Attenuaion Model", attenModStr, (int32_t)attenModStr.size(), &selectedModel))
				{
					spatialConfig->AttenuationMod = static_cast<AttModel>(selectedModel);
				}

				propertyGridSpacing();
				propertyGridSpacing();
				UI::Property("Min Gain", spatialConfig->MinGain, 0.01f, 0.0f, 1.0f);
				UI::Property("Max Gain", spatialConfig->MaxGain, 0.01f, 0.0f, 1.0f);
				UI::Property("Min Distance", spatialConfig->MinDistance, 1.00f, 0.0f, FLT_MAX);
				UI::Property("Max Distance", spatialConfig->MaxDistance, 1.00f, 0.0f, FLT_MAX);

				propertyGridSpacing();
				propertyGridSpacing();

				float inAngle = glm::degrees(spatialConfig->ConeInnerAngleInRadians);
				float outAngle = glm::degrees(spatialConfig->ConeOuterAngleInRadians);
				float outGain = spatialConfig->ConeOuterGain;

				//? Have to manually clamp here because UI::Property doesn't take flags to pass in ImGuiSliderFlags_ClampOnInput
				if (UI::Property("Cone Inner Angle", inAngle, 1.0f, 0.0f, 360.0f))
				{
					if (inAngle > 360.0f) inAngle = 360.0f;
					spatialConfig->ConeInnerAngleInRadians = glm::radians(inAngle);
				}
				if (UI::Property("Cone Outer Angle", outAngle, 1.0f, 0.0f, 360.0f))
				{
					if (outAngle > 360.0f) outAngle = 360.0f;
					spatialConfig->ConeOuterAngleInRadians = glm::radians(outAngle);
				}
				if (UI::Property("Cone Outer Gain", outGain, 0.01f, 0.0f, 1.0f))
				{
					if (outGain > 1.0f) outGain = 1.0f;
					spatialConfig->ConeOuterGain = outGain;
				}

				propertyGridSpacing();
				propertyGridSpacing();
				if (UI::Property("Doppler Factor", spatialConfig->DopplerFactor, 0.01f, 0.0f, 1.0f)) {}
				//if (UI::Property("Rolloff", spatialConfig->Rolloff, 0.01f, 0.0f, 1.0f)) {  }

				propertyGridSpacing();
				propertyGridSpacing();
				// TODO: air absorption filter is not hooked up yet
				//if (UI::Property("Air Absorption", spatialConfig->bAirAbsorptionEnabled)) {  }

				UI::Property("Spread from Source Size", spatialConfig->bSpreadFromSourceSize);
				UI::Property("Source Size", spatialConfig->SourceSize, 0.01f, 0.01f, FLT_MAX);
				UI::Property("Spread", spatialConfig->Spread, 0.01f, 0.0f, 1.0f);
				UI::Property("Focus", spatialConfig->Focus, 0.01f, 0.0f, 1.0f);

				UI::EndPropertyGrid();
				ImGui::TreePop();
			}

			ImGui::Unindent();
		}

		ImGui::EndChild(); // SoundConfigPanel
	}

	PrefabEditor::PrefabEditor()
		: AssetEditor("Prefab Editor"), m_SceneHierarchyPanel(nullptr, SelectionContext::PrefabEditor, false)
	{
	}

	void PrefabEditor::OnOpen()
	{
		SelectionManager::DeselectAll();
	}

	void PrefabEditor::OnClose()
	{
		SelectionManager::DeselectAll(SelectionContext::PrefabEditor);
	}

	void PrefabEditor::Render()
	{
		// Need to do this in order to ensure that the scene hierarchy panel doesn't close immediately.
		// There's been some structural changes since the addition of the PanelManager.
		bool isOpen = true;
		m_SceneHierarchyPanel.OnImGuiRender(isOpen);
	}


	static const char* s_CreateNewSkeletonPopup = "Create New Skeleton";
	static const char* s_CreateNewAnimationPopup = "Create New Animation";
	static const char* s_NoSkeletonPopup = "No Skeleton";
	static const char* s_NoAnimationPopup = "No Animation";
	static const char* s_InvalidMetadataPopup = "Invalid Asset Metadata";


	AnimationControllerEditor::AnimationControllerEditor() : AssetEditor("Animation Controller Editor")
	{
	}


	void AnimationControllerEditor::SetAsset(const Ref<Asset>& asset)
	{
		m_Asset = asset.As<AnimationController>();
		if (!m_Asset)
		{
			SetOpen(false);
			return;
		}
		m_GridColumn0Widths.assign(m_Asset->GetNumStates() + 1, AnimationControllerEditor::FirstColumnWidth); // reset column 0 widths if asset is changed (otherwise, it looks a bit odd if you've resized some grids and then subsequently edit an AnimationController with a different number of states)
	}


	void AnimationControllerEditor::OnOpen()
	{
		if (!m_Asset)
		{
			SetOpen(false);
			return;
		}
		m_GridColumn0Widths.assign(m_Asset->GetNumStates() + 1, AnimationControllerEditor::FirstColumnWidth); // reset column 0 widths every time window is opened  (otherwise, it looks a bit odd if you've resized some grids and then subsequently edit an AnimationController with a different number of states)
	}


	void AnimationControllerEditor::OnClose()
	{
		m_Asset = nullptr;
	}


	void AnimationControllerEditor::Render()
	{
		bool needsSerialize = false;
		int grid = 0;
		UI::BeginPropertyGrid();
		if (m_GridColumn0Widths[grid] > 0.0f)
		{
			ImGui::SetColumnWidth(0, m_GridColumn0Widths[grid]);
			m_GridColumn0Widths[grid] = 0.0f;
		}
		UI::PropertyAssetReferenceError error;
		auto skeletonAsset = m_Asset->GetSkeletonAsset();
		auto skeletonHandle = skeletonAsset ? skeletonAsset->Handle : 0;

		if (UI::PropertyAssetReferenceWithConversion<SkeletonAsset, MeshSource>("Skeleton", skeletonHandle, [=](Ref<MeshSource> meshSource)
			{
				m_ShowCreateNewSkeletonPopup = true;
				m_CreateNewAssetPopupData.MeshSource = meshSource;
			}, &error))
		{
			m_CreateNewAssetPopupData.SkeletonHandle = skeletonHandle;
			m_CreateNewAssetPopupData.GotSkeleton = true;
		}

			if (error == UI::PropertyAssetReferenceError::InvalidMetadata)
			{
				m_ShowInvalidMetadataPopup = true;
				m_InvalidMetadata = Project::GetEditorAssetManager()->GetMetadata(UI::s_PropertyAssetReferenceAssetHandle);
			}

			if (m_CreateNewAssetPopupData.GotSkeleton)
			{
				m_Asset->SetSkeletonAsset(AssetManager::GetAsset<SkeletonAsset>(m_CreateNewAssetPopupData.SkeletonHandle));
				m_CreateNewAssetPopupData.GotSkeleton = false;
				needsSerialize = true;
			}
			UI::EndPropertyGrid();
			++grid;

			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
			{
				UI::ScopedStyle frameBorder(ImGuiStyleVar_FrameBorderSize, 0.0f);
				UI::ScopedColour frameColour(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
				UI::ScopedFont boldFont(ImGui::GetIO().Fonts->Fonts[0]);
				ImGui::Text("States:");
			}
			{
				UI::ScopedColourStack addCompButtonColours(ImGuiCol_Button, IM_COL32(70, 70, 70, 200), ImGuiCol_ButtonHovered, IM_COL32(70, 70, 70, 255), ImGuiCol_ButtonActive, IM_COL32(70, 70, 70, 150));
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImVec2 textSize = ImGui::CalcTextSize(" ADD        ");
				textSize.x += GImGui->Style.FramePadding.x * 2.0f;
				ImGui::SameLine(contentRegionAvailable.x - textSize.x);
				if (ImGui::Button(" ADD       ", ImVec2(textSize.x + 4.0f, lineHeight + 2.0f)))
				{
					m_Asset->AddState("", Ref<AnimationState>::Create());
					m_GridColumn0Widths.emplace_back(AnimationControllerEditor::FirstColumnWidth);
					needsSerialize = true;
				}

				const float pad = 4.0f;
				const float iconWidth = ImGui::GetFrameHeight() - pad * 2.0f;
				const float iconHeight = iconWidth;
				ImVec2 iconPos = ImGui::GetItemRectMax();
				iconPos.x -= iconWidth + pad;
				iconPos.y -= iconHeight + pad;
				ImGui::SetCursorScreenPos(iconPos);
				UI::ShiftCursor(-5.0f, -1.0f);
				UI::Image(EditorResources::PlusIcon, ImVec2(iconWidth, iconHeight), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImColor(160, 160, 160, 255).Value);
			}
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			size_t removeState = ~0;
			for (size_t i = 0; i < m_Asset->GetNumStates(); ++i)
			{
				UI::ScopedID id(static_cast<int>(i));
				auto state = m_Asset->GetAnimationState(i);
				contentRegionAvailable = ImGui::GetContentRegionAvail();
				bool open = UI::PropertyGridHeader(fmt::format("{}", i).c_str());
				bool rightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
				float lineHeight = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;

				ImGui::SameLine(contentRegionAvailable.x - lineHeight);
				if (ImGui::InvisibleButton("##options", ImVec2{ lineHeight, lineHeight }) || rightClicked)
				{
					ImGui::OpenPopup("Options");
				}
				UI::DrawButtonImage(EditorResources::GearIcon, IM_COL32(160, 160, 160, 200), IM_COL32(160, 160, 160, 255), IM_COL32(160, 160, 160, 150), UI::RectExpanded(UI::GetItemRect(), -6.0f, -6.0f));

				if (UI::BeginPopup("Options"))
				{
					if (ImGui::MenuItem("Remove state"))
						removeState = i;

					UI::EndPopup();
				}

				if (open)
				{
					UI::BeginPropertyGrid();
					if (m_GridColumn0Widths[grid] > 0.0f)
					{
						ImGui::SetColumnWidth(0, m_GridColumn0Widths[grid]);
						m_GridColumn0Widths[grid] = 0.0f;
					}

					std::string name = m_Asset->GetStateName(i);
					if (UI::Property("Name", name))
					{
						m_Asset->SetStateName(i, name);
						needsSerialize = true;
					}

					auto animationAsset = state->GetAnimationAsset();
					auto animationHandle = animationAsset ? animationAsset->Handle : 0;

					UI::PropertyAssetReferenceSettings settings;
					if (!state->IsValid())
					{
						settings.ButtonLabelColor = ImGui::ColorConvertU32ToFloat4(Colours::Theme::textError);
					}

					if (UI::PropertyAssetReferenceWithConversion<AnimationAsset, MeshSource>("Animation", animationHandle, [=](Ref<MeshSource> meshSource)
						{
							m_ShowCreateNewAnimationPopup = true;
							m_CreateNewAssetPopupData.MeshSource = meshSource;
							m_CreateNewAssetPopupData.State = (uint32_t)i;
						}, &error, settings))
					{
						m_CreateNewAssetPopupData.AnimationHandle = animationHandle;
						m_CreateNewAssetPopupData.State = (uint32_t)i;
						m_CreateNewAssetPopupData.GotAnimation = true;
					}

						if (error == UI::PropertyAssetReferenceError::InvalidMetadata)
						{
							m_ShowInvalidMetadataPopup = true;
							m_InvalidMetadata = Project::GetEditorAssetManager()->GetMetadata(UI::s_PropertyAssetReferenceAssetHandle);
						}

						if (m_CreateNewAssetPopupData.GotAnimation && (m_CreateNewAssetPopupData.State == i))
						{
							animationAsset = AssetManager::GetAsset<AnimationAsset>(m_CreateNewAssetPopupData.AnimationHandle);
							state->SetAnimationAsset(animationAsset);
							m_CreateNewAssetPopupData.GotAnimation = false;
							needsSerialize = true;
						}

						uint32_t animationIndex = state->GetAnimationIndex();
						if (UI::Property("Animation Index", animationIndex, 0, animationAsset ? animationAsset->GetAnimationCount() - 1 : 0))
						{
							state->SetAnimationIndex(animationIndex);
							needsSerialize = true;
						}

						auto loop = state->IsLooping();
						if (UI::Property("Loop", loop))
						{
							state->SetIsLooping(loop);
							needsSerialize = true;
						}

						auto translationExtract = state->GetRootTranslationExtractMask();
						if (UI::Property("Root Translation Extract", translationExtract, 1.0f, 0.0f, 1.0f, "TEMPORARY: These components of root bone translation will be extracted as the root motion.  For example, if you only want to extract the forwards movement of root bone as root motion, then set Z to one, and X and Y to zero."))
						{
							state->SetRootTranslationExtractMask(translationExtract);
							needsSerialize = true;
						}

						auto translationMask = state->GetRootTranslationMask();
						if (UI::Property("Root Translation Mask", translationMask, 1.0f, 0.0f, 1.0f, "TEMPORARY: These components of root bone translation will be masked out.  You should mask out at least the components that you are extracting as root motion (but you can mask out other components as well (e.g. if you want to suppress some of the movement)."))
						{
							state->SetRootTranslationMask(translationMask);
							needsSerialize = true;
						}

						auto rotationExtract = state->GetRootRotationExtractMask();
						if (UI::Property("Root Rotation Extract", rotationExtract, 1.0f, 0.0f, 1.0f, "TEMPORARY: Set to one to extract root bone's rotation about Y-axis as root motion.  Set to zero to ignore rotation for root motion."))
						{
							state->SetRootRotationExtractMask(rotationExtract);
							needsSerialize = true;
						}

						auto rotationMask = state->GetRootRotationExtractMask();
						if (UI::Property("Root Rotation Mask", rotationMask, 1.0f, 0.0f, 1.0f, "TEMPORARY: Set to one to mask out the root bone's rotation about Y-axis.  If you are extracting root bone rotation about Y-axis, then you should also mask it. If you are not extracting root bone rotation, you can set this to 1 anyway to suppress rotation about Y-axis."))
						{
							state->SetRootRotationMask(rotationExtract);
							needsSerialize = true;
						}

						// If the skeleton asset, animation asset, or index into animation asset has changed (needsSerialize will be true in this case),
						// then we need to check that the animation is still "compatible" with the skeleton.
						if (needsSerialize)
						{
							state->SetValid(
								animationAsset &&
								animationAsset->GetMeshSource() &&
								animationAsset->GetMeshSource()->IsValid() &&
								m_Asset->GetSkeletonAsset() &&
								m_Asset->GetSkeletonAsset()->IsValid() &&
								animationAsset->GetMeshSource()->IsCompatibleSkeleton(animationIndex, m_Asset->GetSkeletonAsset()->GetSkeleton())
							);
						}

						UI::EndPropertyGrid();
						++grid;
						ImGui::TreePop();
				}
			}
			if (removeState != ~0)
			{
				m_Asset->RemoveState(removeState);
				needsSerialize = true;
			}
			UI_CreateNewSkeletonPopup();
			UI_CreateNewAnimationPopup();
			UI_NoSkeletonPopup();
			UI_NoAnimationPopup();
			UI_InvalidMetadataPopup();
			if (needsSerialize)
			{
				ANT_CORE_WARN("Serializing...");
				AssetImporter::Serialize(m_Asset);
			}
	}


	void AnimationControllerEditor::UI_CreateNewSkeletonPopup()
	{
		float gridColumn0Width = 0.0f;
		const std::string assetPathLabel = fmt::format("{0}/", Project::GetActive()->GetConfig().AnimationPath);

		if (m_ShowCreateNewSkeletonPopup)
		{
			float maxAssetPathWidth = std::max({ 100.0f, ImGui::CalcTextSize(assetPathLabel.c_str()).x }) + 20.0f;
			gridColumn0Width = maxAssetPathWidth;

			ImGui::OpenPopup(s_CreateNewSkeletonPopup);
			m_ShowCreateNewSkeletonPopup = false;
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2{ 600,0 });
		if (ImGui::BeginPopupModal(s_CreateNewSkeletonPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped("A Skeleton asset must be created from this Mesh Source file (e.g. FBX) before it can be added to the Animation Controller. More options can be accessed by double-clicking a Mesh Source file in the Content Browser panel.");

			ImGui::AlignTextToFramePadding();

			const AssetMetadata& assetData = Project::GetEditorAssetManager()->GetMetadata(m_CreateNewAssetPopupData.MeshSource->Handle);
			std::string filepathSkeleton = fmt::format("{0}.{1}", assetData.FilePath.stem().string(), "askel");

			if (!m_CreateNewAssetPopupData.CreateAssetFilenameBuffer[0])
			{
				std::copy(filepathSkeleton.begin(), filepathSkeleton.end(), m_CreateNewAssetPopupData.CreateAssetFilenameBuffer.begin());
			}

			ImGui::Text("Asset Path:");
			UI::Draw::Underline();
			{
				UI::BeginPropertyGrid();
				if (gridColumn0Width > 0.0f)
				{
					ImGui::SetColumnWidth(0, gridColumn0Width);
					gridColumn0Width = 0.0f;
				}
				UI::Property(assetPathLabel.c_str(), m_CreateNewAssetPopupData.CreateAssetFilenameBuffer.data(), 256);
				UI::EndPropertyGrid();
			}

			ANT_CORE_ASSERT(m_CreateNewAssetPopupData.MeshSource);

			if (ImGui::Button("Create"))
			{
				if (m_CreateNewAssetPopupData.MeshSource->HasSkeleton())
				{
					std::string serializePath = m_CreateNewAssetPopupData.CreateAssetFilenameBuffer.data();
					std::filesystem::path path = Project::GetActive()->GetAnimationPath() / serializePath;
					if (!FileSystem::Exists(path.parent_path()))
						FileSystem::CreateDirectory(path.parent_path());

					m_CreateNewAssetPopupData.SkeletonHandle = Project::GetEditorAssetManager()->CreateNewAsset<SkeletonAsset>(path.filename().string(), path.parent_path().string(), m_CreateNewAssetPopupData.MeshSource)->Handle;
					m_CreateNewAssetPopupData.GotSkeleton = true;
				}
				else
				{
					m_ShowNoSkeletonPopup = true;
				}

				m_CreateNewAssetPopupData.Reset();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				m_CreateNewAssetPopupData.Reset();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}


	void AnimationControllerEditor::UI_CreateNewAnimationPopup()
	{
		float gridColumn0Width = 0.0f;
		const std::string assetPathLabel = fmt::format("{0}/", Project::GetActive()->GetConfig().AnimationPath);

		if (m_ShowCreateNewAnimationPopup)
		{
			float maxAssetPathWidth = std::max({ 100.0f, ImGui::CalcTextSize(assetPathLabel.c_str()).x }) + 20.0f;
			gridColumn0Width = maxAssetPathWidth;

			ImGui::OpenPopup(s_CreateNewAnimationPopup);
			m_ShowCreateNewAnimationPopup = false;
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2{ 600,0 });
		if (ImGui::BeginPopupModal(s_CreateNewAnimationPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped("An Animation asset must be created from this Mesh Source file (eg. FBX) before it can be added to the Animation Controller. More options can be accessed by double-clicking a Mesh Source file in the Content Browser panel.");

			ImGui::AlignTextToFramePadding();

			const AssetMetadata& assetData = Project::GetEditorAssetManager()->GetMetadata(m_CreateNewAssetPopupData.MeshSource->Handle);
			std::string filepathAnimation = fmt::format("{0}.{1}", assetData.FilePath.stem().string(), "aanim");

			if (!m_CreateNewAssetPopupData.CreateAssetFilenameBuffer[0])
			{
				std::copy(filepathAnimation.begin(), filepathAnimation.end(), m_CreateNewAssetPopupData.CreateAssetFilenameBuffer.begin());
			}

			ImGui::Text("Asset Path:");
			UI::Draw::Underline();
			{
				UI::BeginPropertyGrid();
				if (gridColumn0Width > 0.0f)
				{
					ImGui::SetColumnWidth(0, gridColumn0Width);
					gridColumn0Width = 0.0f;
				}
				UI::Property(assetPathLabel.c_str(), m_CreateNewAssetPopupData.CreateAssetFilenameBuffer.data(), 256);
				UI::EndPropertyGrid();
			}

			ANT_CORE_ASSERT(m_CreateNewAssetPopupData.MeshSource);

			if (ImGui::Button("Create"))
			{
				if (m_CreateNewAssetPopupData.MeshSource->GetAnimationCount() > 0)
				{
					std::string serializePath = m_CreateNewAssetPopupData.CreateAssetFilenameBuffer.data();
					std::filesystem::path path = Project::GetActive()->GetAnimationPath() / serializePath;
					if (!FileSystem::Exists(path.parent_path()))
						FileSystem::CreateDirectory(path.parent_path());

					m_CreateNewAssetPopupData.AnimationHandle = Project::GetEditorAssetManager()->CreateNewAsset<AnimationAsset>(path.filename().string(), path.parent_path().string(), m_CreateNewAssetPopupData.MeshSource)->Handle;
					m_CreateNewAssetPopupData.GotAnimation = true;
				}
				else
				{
					m_ShowNoAnimationPopup = true;
				}

				m_CreateNewAssetPopupData.Reset();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				m_CreateNewAssetPopupData.Reset();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void AnimationControllerEditor::UI_NoSkeletonPopup()
	{
		if (m_ShowNoSkeletonPopup && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId))
		{
			ImGui::OpenPopup(s_NoSkeletonPopup);
			m_ShowNoSkeletonPopup = false;
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2{ 400,0 });
		if (ImGui::BeginPopupModal(s_NoSkeletonPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped("A skeleton asset was not created because no skeletal animation rig was found in the source asset.  (Note: if the source asset does contain a skeleton but is unskinned (i.e. no mesh), then Assimp will not load that skeleton).");
			if (ImGui::Button("OK"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void AnimationControllerEditor::UI_NoAnimationPopup()
	{
		if (m_ShowNoAnimationPopup && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId))
		{
			ImGui::OpenPopup(s_NoAnimationPopup);
			m_ShowNoAnimationPopup = false;
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2{ 400,0 });
		if (ImGui::BeginPopupModal(s_NoAnimationPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped("An animation asset was not created because no animations were found in the source asset.");
			if (ImGui::Button("OK"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void AnimationControllerEditor::UI_InvalidMetadataPopup()
	{
		if (m_ShowInvalidMetadataPopup)
		{
			ImGui::OpenPopup(s_InvalidMetadataPopup);
			m_ShowInvalidMetadataPopup = false;
		}

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2{ 400,0 });
		if (ImGui::BeginPopupModal(s_InvalidMetadataPopup, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::TextWrapped("You tried to use an asset with invalid metadata. This can happen when an asset has a reference to another non-existent asset, or when as asset is empty.");
			ImGui::Separator();

			UI::BeginPropertyGrid();
			const auto& filepath = m_InvalidMetadata.FilePath.string();
			UI::Property("Asset Filepath", filepath);
			UI::Property("Asset ID", fmt::format("{0}", (uint64_t)m_InvalidMetadata.Handle));
			UI::EndPropertyGrid();

			if (ImGui::Button("OK"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}
}