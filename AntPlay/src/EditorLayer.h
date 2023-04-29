#pragma once
#include <Ant.h>

namespace Ant {
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(Timestep ts);
		virtual void OnImGuiRender() override;
		void OnEvent(Event& event);

	private:
		Ant::OrthographicCameraController m_CameraController;

		// Temp
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_Shader;
		Ref<Framebuffer> m_Framebuffer;

		Ref<Texture2D> m_Texture;

		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		glm::vec4 m_Color = { 0.8f, 0.3f, 0.2f, 1.0f };
	};
}

