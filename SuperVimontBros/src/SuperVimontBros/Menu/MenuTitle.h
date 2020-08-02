#pragma once

#include "Menu.h"

namespace sf
{
	class RenderTexture;
}

class GameStateData;

static const float title_fontSize = 8;
static const sf::Color title_enabledColor = { 255,255,255,255 };
static const sf::Color title_disabledColor = { 255,255,255,128 };
static const sf::Color title_unavailableColor = { 128,128,128,64 };
static const float title_joyDeadZone = 50;

class MenuTitle : public Menu
{
	enum class TitleMenu
	{
		Play = 0,
		Credits,

		Count
	};

public:
	void update(GameStateData & _data);
	void draw(sf::RenderTexture & _dst);

	void menuUp();
	void menuDown();
	void menuButton(unsigned int _button);

	TitleMenu m_titleMenu = TitleMenu::Play;

	sf::Texture	m_title;
};