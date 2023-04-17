#pragma once
#include <Ant.h>

class Sandbox2D : public Ant::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Ant::Timestep ts);
	virtual void OnImGuiRender() override;
	void OnEvent(Ant::Event& event);

private:
	Ant::OrthographicCameraController m_CameraController;

	// Temp
	Ant::Ref<Ant::Shader> m_Shader;
	Ant::Ref<Ant::VertexArray> m_SquareVA;
	glm::vec4 m_Color = { 0.8f, 0.3f, 0.2f, 1.0f };
};

