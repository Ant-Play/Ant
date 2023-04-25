#pragma once
#include "Ant.h"
#include "Level.h"


#include <imgui.h>


class GameLayer : public Ant::Layer
{
public:
	GameLayer();
	virtual ~GameLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Ant::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Ant::Event& event) override;
	bool OnMouseButtonPressed(Ant::MouseButtonPressedEvent& e);
	bool OnWindowResize(Ant::WindowResizeEvent& e);
private:
	void CreateCamera(uint32_t width, uint32_t height);
private:
	Ant::Scope<Ant::OrthographicCamera> m_Camera;
	Level m_Level;
	ImFont* m_Font;
	float m_Time = 0.0f;
	bool m_Blink = false;

	enum class GameState
	{
		Play = 0, MainMenu = 1, GameOver = 2
	};

	GameState m_State = GameState::MainMenu;
};
