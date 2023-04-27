#pragma once
#include "Ant.h"


class ExampleLayer : public Ant::Layer
{
	ExampleLayer();

	void OnUpdate(Ant::Timestep ts);
	virtual void OnImGuiRender() override;
	void OnEvent(Ant::Event& event);

private:
	Ant::ShaderLibrary m_ShaderLibrary;
	Ant::Ref<Ant::Texture2D> m_Texture, m_AnotherTexture;
	Ant::Ref<Ant::Shader> m_Shader, tmpShader;
	Ant::Ref<Ant::VertexArray> m_VertexArray, tmpVA;
	Ant::Ref<Ant::VertexBuffer> m_VertexBuffer;
	Ant::Ref<Ant::IndexBuffer> m_IndexBuffer;

	Ant::OrthographicCameraController m_CameraController;

	glm::vec3 m_TmpColor = { 0.8, 0.3, 0.2 };
};
