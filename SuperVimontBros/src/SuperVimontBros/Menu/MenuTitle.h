#pragma once

#include "Menu.h"

static const float title_fontSize = 8;
static const sf::Color title_enabledColor = { 255,255,255,255 };
static const sf::Color title_disabledColor = { 255,255,255,128 };
static const sf::Color title_unavailableColor = { 128,128,128,64 };
static const float title_joyDeadZone = 50;

class MenuTitle : public Menu
{
public:
	enum class SubMenu
	{
		Main = 0,

		Players,
		Credits,

		Count
	};

	MenuTitle();
	~MenuTitle();

	void init(bool _reinit);

	void setTitleMenu(SubMenu _menu);

	void update(float _dt, GameStateData & _data) override;
	void draw(sf::RenderTexture & _dst) override;

	void onMenuLeft(uint _player) override;
	void onMenuRight(uint _player) override;
	void onMenuUp(uint _player) override;
	void onMenuDown(uint _player) override;
	void onMenuAction(uint _player, unsigned int _button) override;

private:
	void drawTitleLogo(sf::RenderTexture & _dst);

private:
	SubMenu		m_titleMenu = SubMenu::Players;
	sf::Texture	m_title;
	Menu *		m_subMenu[(uint)SubMenu::Count];
	Menu *		m_selected = nullptr;
};