#pragma once
#include <Ant.h>

#include "ParticleSystem.h"
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
	Ant::Ref<Ant::VertexArray> m_SquareVA;
	Ant::Ref<Ant::Shader> m_Shader;

	Ant::Ref<Ant::Texture2D> m_Texture;
	Ant::Ref<Ant::Texture2D> m_SpriteSheet;
	Ant::Ref<Ant::SubTexture2D> m_TextureStairs, m_TextureDirt, m_TextureTree;

	glm::vec4 m_Color = { 0.8f, 0.3f, 0.2f, 1.0f };

	ParticleSystem m_ParticleSystem;
	ParticleProps m_Particle;

	uint32_t m_MapWidth, m_MapHeight;
	std::unordered_map<char, Ant::Ref<Ant::SubTexture2D>> s_TextureMap;
};

