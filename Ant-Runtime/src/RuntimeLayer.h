#pragma once

#include <Ant/Asset/Asset.h>
#include <Ant/Core/Application.h>
#include <Ant/Core/Layer.h>
#include <Ant/Core/Ref.h >
#include <Ant/Core/TimeStep.h>
#include <Ant/Core/Events/Event.h>
#include <Ant/Core/Events/KeyEvent.h>
#include <Ant/Core/Events/MouseEvent.h>
#include <Ant/Renderer/Material.h>
#include <Ant/Renderer/Renderer2D.h>
#include <Ant/Renderer/RenderCommandBuffer.h>
#include <Ant/Renderer/Pipeline.h>
#include <Ant/Renderer/SceneRenderer.h>
#include <Ant/Scene/Prefab.h>
#include <Ant/Scene/Scene.h>

#include <glm/glm.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace Ant {

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer(std::string_view projectPath);
		virtual ~RuntimeLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;

		virtual void OnEvent(Event& e) override;
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void OpenProject();
		void OpenScene(const std::string& filepath);

		void LoadScene(AssetHandle sceneHandle);
	private:
		void OnScenePlay();
		void OnSceneStop();

		void QueueSceneTransition(const std::string& scene);
		void OnSceneTransition(const std::string& scene);

		void UpdateWindowTitle(const std::string& sceneName);

		void LoadSceneAssets();
		void LoadPrefabAssets(Ref<Prefab> prefab);

		void DrawDebugStats();
		void DrawString(const std::string& string, const glm::vec2& position, const glm::vec4& color = glm::vec4(1.0f), float size = 50.0f);
		void UpdateFPSStat();
		void UpdatePerformanceTimers();
	private:
		Ref<Scene> m_RuntimeScene;
		Ref<SceneRenderer> m_SceneRenderer;
		Ref<Renderer2D> m_Renderer2D;

		Ref<RenderCommandBuffer> m_CommandBuffer;
		Ref<Pipeline> m_SwapChainPipeline;
		Ref<Material> m_SwapChainMaterial;

		std::string m_ProjectPath;
		bool m_ReloadScriptOnPlay = true;

		std::vector<std::function<void()>> m_PostSceneUpdateQueue;

		glm::mat4 m_Renderer2DProj;

		uint32_t m_Width = 0, m_Height = 0;
		uint32_t m_FramesPerSecond = 0;
		float m_UpdateFPSTimer = 0.0f;
		float m_UpdatePerformanceTimer = 0.0f;

		Application::PerformanceTimers m_PerformanceTimers;
		float m_FrameTime = 0.0f;
		float m_GPUTime = 0.0f;

		// For debugging
		EditorCamera m_EditorCamera;

		bool m_AllowViewportCameraEvents = false;
		bool m_DrawOnTopBoundingBoxes = false;

		bool m_UIShowBoundingBoxes = false;
		bool m_UIShowBoundingBoxesOnTop = false;

		bool m_ViewportPanelMouseOver = false;
		bool m_ViewportPanelFocused = false;

		bool m_ShowPhysicsSettings = false;

		bool m_ShowDebugDisplay = false;
	};
}