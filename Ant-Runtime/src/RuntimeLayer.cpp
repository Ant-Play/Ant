#include "RuntimeLayer.h"

#include <Ant/Core/Inputs.h>
#include <Ant/Renderer/Renderer.h>
#include <Ant/Project/Project.h>
#include <Ant/Project/ProjectSerializer.h>
#include <Ant/Scene/SceneSerializer.h>

#include <Ant/Serialization/AssetPack.h>

//#include "../../Ant-Launcher/src/Ant/TieringSerializer.h"

namespace Ant {

	static bool s_AudioDisabled = true;

	RuntimeLayer::RuntimeLayer(std::string_view projectPath)
		: m_EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 1000.0f)
		, m_ProjectPath(projectPath)
	{
	}

	RuntimeLayer::~RuntimeLayer()
	{
	}

	void RuntimeLayer::OnAttach()
	{
		OpenProject();

		SceneRendererSpecification spec;

		Tiering::TieringSettings ts;

		//		if (TieringSerializer::Deserialize(ts, "Forest/Settings.yaml"))
		//			spec.Tiering = ts.RendererTS;

				// Some overrides
				// spec.Tiering.RendererScale = 0.67f;
				// spec.Tiering.ShadowQuality = Tiering::Renderer::ShadowQualitySetting::Low;
				// spec.Tiering.EnableAO = false;
		m_SceneRenderer = Ref<SceneRenderer>::Create(m_RuntimeScene, spec);
		m_SceneRenderer->GetOptions().ShowGrid = false;
		m_SceneRenderer->SetShadowSettings(-15.0f, 15.0f, 0.93f);
		m_SceneRenderer->SetShadowSettings(-50.0f, 50.0f, 0.92f);

		m_Renderer2D = Ref<Renderer2D>::Create();
		m_Renderer2D->SetLineWidth(2.0f);

		// Setup swapchain render pass
		FramebufferSpecification compFramebufferSpec;
		compFramebufferSpec.DebugName = "SceneComposite";
		compFramebufferSpec.ClearColor = { 0.5f, 0.1f, 0.1f, 1.0f };
		compFramebufferSpec.SwapChainTarget = true;
		compFramebufferSpec.Attachments = { ImageFormat::RGBA };

		Ref<Framebuffer> framebuffer = Framebuffer::Create(compFramebufferSpec);

		PipelineSpecification pipelineSpecification;
		pipelineSpecification.Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		pipelineSpecification.BackfaceCulling = false;
		pipelineSpecification.Shader = Renderer::GetShaderLibrary()->Get("TexturePass");

		RenderPassSpecification renderPassSpec;
		renderPassSpec.TargetFramebuffer = framebuffer;
		renderPassSpec.DebugName = "SceneComposite";
		pipelineSpecification.RenderPass = RenderPass::Create(renderPassSpec);
		pipelineSpecification.DebugName = "SceneComposite";
		pipelineSpecification.DepthWrite = false;
		m_SwapChainPipeline = Pipeline::Create(pipelineSpecification);

		m_SwapChainMaterial = Material::Create(pipelineSpecification.Shader);

		m_CommandBuffer = RenderCommandBuffer::CreateFromSwapChain("RuntimeLayer");

		OnScenePlay();
	}

	void RuntimeLayer::OnDetach()
	{
		OnSceneStop();

		ScriptEngine::SetSceneContext(nullptr, nullptr);
		m_SceneRenderer->SetScene(nullptr);

		ANT_CORE_VERIFY(m_RuntimeScene->GetRefCount() == 1);
		m_RuntimeScene = nullptr;
	}

	void RuntimeLayer::OnScenePlay()
	{
		m_RuntimeScene->SetSceneTransitionCallback([this](const std::string& scene) { QueueSceneTransition(scene); });
		ScriptEngine::SetSceneContext(m_RuntimeScene, m_SceneRenderer);
		m_RuntimeScene->OnRuntimeStart();
	}

	void RuntimeLayer::OnSceneStop()
	{
		m_RuntimeScene->OnRuntimeStop();
	}

	void RuntimeLayer::QueueSceneTransition(const std::string& scene)
	{
		m_PostSceneUpdateQueue.emplace_back([this, scene]() { OnSceneTransition(scene); });
	}

	void RuntimeLayer::OnSceneTransition(const std::string& scene)
	{
		Ref<Scene> newScene = Ref<Scene>::Create();
		SceneSerializer serializer(newScene);
		if (serializer.Deserialize((Project::GetAssetDirectory() / scene).string()))
		{
			m_RuntimeScene->OnRuntimeStop();
			m_RuntimeScene = newScene;
			LoadSceneAssets();

			m_RuntimeScene->SetSceneTransitionCallback([this](const std::string& scene) { QueueSceneTransition(scene); });
			ScriptEngine::SetSceneContext(m_RuntimeScene, m_SceneRenderer);
			m_RuntimeScene->OnRuntimeStart();
		}
		else
		{
			ANT_CORE_ERROR("Could not deserialize scene {0}", scene);
		}
	}

	void RuntimeLayer::UpdateWindowTitle(const std::string& sceneName)
	{
		Application::Get().GetWindow().SetTitle(sceneName);
	}

	void RuntimeLayer::LoadSceneAssets()
	{
	}
#if 0
	void RuntimeLayer::LoadSceneAssets()
	{
		auto assetList = m_RuntimeScene->GetAssetList();
		for (AssetHandle handle : assetList)
		{
			const AssetMetadata& metadata = AssetManager::GetMetadata(handle);
			AssetManager::ReloadData(handle);
			if (metadata.Type == AssetType::Prefab)
			{
				Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(handle);
				LoadPrefabAssets(prefab);
			}
		}
	}

	void RuntimeLayer::LoadPrefabAssets(Ref<Prefab> prefab)
	{
		auto assetList = prefab->GetAssetList();
		for (AssetHandle handle : assetList)
		{
			const AssetMetadata& metadata = AssetManager::GetMetadata(handle);
			AssetManager::ReloadData(handle);
			if (metadata.Type == AssetType::Prefab)
			{
				Ref<Prefab> prefab = AssetManager::GetAsset<Prefab>(handle);
				LoadPrefabAssets(prefab);
			}
		}
	}
#endif

	void RuntimeLayer::LoadPrefabAssets(Ref<Prefab> prefab)
	{
	}


	void RuntimeLayer::DrawDebugStats()
	{
		auto& app = Application::Get();

		m_Renderer2D->SetTargetRenderPass(m_SceneRenderer->GetExternalCompositeRenderPass());
		m_Renderer2D->BeginScene(m_Renderer2DProj, glm::mat4(1.0f));

		// Add font size to this after each line
		float y = 30.0f;

		float fontSize = 25.0f;

		if (false)
		{
			const auto& perFrameData = app.GetProfilerPreviousFrameData();
			for (const auto& [name, time] : perFrameData)
			{
				DrawString(fmt::format("{}: {:.3f} ms", name, time), { 30.0f, y }, glm::vec4(1.0f), fontSize);
				y += fontSize;
			}
		}

		fontSize = 30.0f;

		DrawString(fmt::format("{:.2f} ms ({:.2f}ms GPU)", m_FrameTime, m_GPUTime), { 30.0f, y }, glm::vec4(1.0f), fontSize);
		y += fontSize;
		DrawString(fmt::format("Render Thread {:.2f} ms ({:.2f}ms wait)", m_PerformanceTimers.RenderThreadWorkTime, m_PerformanceTimers.RenderThreadWaitTime), { 30.0f, y }, glm::vec4(1.0f), fontSize);
		y += fontSize;
		DrawString(fmt::format("Main Thread {:.2f} ms ({:.2f}ms wait)", m_PerformanceTimers.MainThreadWorkTime, m_PerformanceTimers.MainThreadWaitTime), { 30.0f, y }, glm::vec4(1.0f), fontSize);
		y += fontSize;
		DrawString(fmt::format("{} fps", (uint32_t)m_FramesPerSecond), { 30.0f, y }, glm::vec4(1.0f), fontSize);
		y += fontSize;
		DrawString(fmt::format("{} entities", (uint32_t)m_RuntimeScene->GetEntityMap().size()), { 30.0f, y }, glm::vec4(1.0f), fontSize);
		y += fontSize;
		DrawString(fmt::format("{} script entities", (uint32_t)ScriptEngine::GetEntityInstances().size()), { 30.0f, y }, glm::vec4(1.0f), fontSize);
		y += fontSize;
		DrawString(fmt::format("{} ({})", m_RuntimeScene->GetName(), (uint64_t)m_RuntimeScene->GetUUID()), { 30.0f, y }, glm::vec4(1.0f), fontSize * 0.8f);
		y += fontSize * 0.8f;

		m_Renderer2D->EndScene();
	}

	void RuntimeLayer::DrawString(const std::string& string, const glm::vec2& position, const glm::vec4& color, float size)
	{
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(size));
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), { position.x, position.y, -0.2f }) * scale;
		m_Renderer2D->DrawString(string, Font::GetDefaultFont(), transform, 1000.0f);
	}

	void RuntimeLayer::UpdateFPSStat()
	{
		auto& app = Application::Get();
		m_FramesPerSecond = 1.0f / (float)app.GetFrametime();
	}

	void RuntimeLayer::UpdatePerformanceTimers()
	{
		auto& app = Application::Get();
		m_FrameTime = (float)app.GetFrametime().GetMilliseconds();
		m_PerformanceTimers = app.GetPerformanceTimers();
		m_GPUTime = m_SceneRenderer->GetStatistics().TotalGPUTime;
	}

	void RuntimeLayer::OnUpdate(Timestep ts)
	{
		m_UpdateFPSTimer -= ts;
		if (m_UpdateFPSTimer <= 0.0f)
		{
			UpdateFPSStat();
			m_UpdateFPSTimer = 1.0f;
		}

		m_UpdatePerformanceTimer -= ts;
		if (m_UpdatePerformanceTimer <= 0.0f)
		{
			UpdatePerformanceTimers();
			m_UpdatePerformanceTimer = 0.2f;
		}

		auto& app = Application::Get();

		auto [width, height] = app.GetWindow().GetSize();
		m_SceneRenderer->SetViewportSize(width, height);
		m_RuntimeScene->SetViewportSize(width, height);
		m_EditorCamera.SetViewportSize(width, height);
		m_Renderer2DProj = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

		if (m_Width != width || m_Height != height)
		{
			m_Width = width;
			m_Height = height;
			m_Renderer2D->OnRecreateSwapchain();

			// Retrieve new main command buffer
			m_CommandBuffer = RenderCommandBuffer::CreateFromSwapChain("RuntimeLayer");
		}

		if (m_ViewportPanelFocused)
			m_EditorCamera.OnUpdate(ts);

		m_RuntimeScene->OnUpdateRuntime(ts);
		m_RuntimeScene->OnRenderRuntime(m_SceneRenderer, ts);

		if (m_ShowDebugDisplay)
			DrawDebugStats();

		// Render final image to swapchain
		Ref<Image2D> finalImage = m_SceneRenderer->GetExternalCompositeRenderPass()->GetSpecification().TargetFramebuffer->GetImage();
		if (finalImage)
		{
			m_SwapChainMaterial->Set("u_Texture", finalImage);

			m_CommandBuffer->Begin();
			Renderer::BeginRenderPass(m_CommandBuffer, m_SwapChainPipeline->GetSpecification().RenderPass);
			Renderer::SubmitFullscreenQuad(m_CommandBuffer, m_SwapChainPipeline, nullptr, m_SwapChainMaterial);
			Renderer::EndRenderPass(m_CommandBuffer);
			m_CommandBuffer->End();
		}
		else
		{
			// Clear render pass if no image is present
			m_CommandBuffer->Begin();
			Renderer::BeginRenderPass(m_CommandBuffer, m_SwapChainPipeline->GetSpecification().RenderPass);
			Renderer::EndRenderPass(m_CommandBuffer);
			m_CommandBuffer->End();
		}

		for (auto& fn : m_PostSceneUpdateQueue)
			fn();
		m_PostSceneUpdateQueue.clear();
	}

	void RuntimeLayer::OpenProject()
	{
		Ref<Project> project = Ref<Project>::Create();
		ProjectSerializer serializer(project);
		serializer.Deserialize(m_ProjectPath);


		// Load asset pack
		std::filesystem::path defaultAssetPackPath = std::filesystem::path(project->GetConfig().ProjectDirectory) / project->GetConfig().AssetDirectory / "AssetPack.aap";
		Ref<AssetPack> assetPack = AssetPack::Load(defaultAssetPackPath);
		Project::SetActiveRuntime(project, assetPack);

		//if (!s_AudioDisabled)
		//	AudioCommandRegistry::Init();

		Buffer appBinary = assetPack->ReadAppBinary();
		ScriptEngine::LoadAppAssemblyRuntime(appBinary);
		appBinary.Release();

		// Reset cameras
		m_EditorCamera = EditorCamera(45.0f, 1280.0f, 720.0f, 0.1f, 1000.0f);


		// Change scene UUID here!
		LoadScene(5335834865867409016); // PrefabSandbox=5335834865867409016
		//LoadScene(8428220783520776882); // SponzaDemo=8428220783520776882

		//if (!project->GetConfig().StartScene.empty())
		//	OpenScene((Project::GetAssetDirectory() / project->GetConfig().StartScene).string());

	}

	void RuntimeLayer::OpenScene(const std::string& filepath)
	{
		ANT_CORE_VERIFY(false); // Not supported in runtime
		Ref<Scene> newScene = Ref<Scene>::Create("New Scene", false);
		SceneSerializer serializer(newScene);
		serializer.Deserialize(filepath);
		m_RuntimeScene = newScene;
		LoadSceneAssets();

		std::filesystem::path path = filepath;
		//UpdateWindowTitle(path.filename().string());
		ScriptEngine::SetSceneContext(m_RuntimeScene, m_SceneRenderer);
	}

	void RuntimeLayer::LoadScene(AssetHandle sceneHandle)
	{
		Ref<Scene> scene = Project::GetRuntimeAssetManager()->LoadScene(sceneHandle);
		m_RuntimeScene = scene;
		ScriptEngine::SetSceneContext(m_RuntimeScene, m_SceneRenderer);
	}

	void RuntimeLayer::OnEvent(Event& e)
	{
		m_RuntimeScene->OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ANT_BIND_EVENT_FN(RuntimeLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseButtonPressedEvent>(ANT_BIND_EVENT_FN(RuntimeLayer::OnMouseButtonPressed));
	}

	bool RuntimeLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
			case KeyCode::Escape:
				break;
		}

		if (e.GetRepeatCount() == 0 && Input::IsKeyDown(KeyCode::LeftControl))
		{
			switch (e.GetKeyCode())
			{
				case KeyCode::F3:
					m_ShowDebugDisplay = !m_ShowDebugDisplay;
					break;
			}
		}

		return false;
	}

	bool RuntimeLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		return false;
	}
}