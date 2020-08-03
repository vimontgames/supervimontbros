#pragma once

#include "MenuTitle.h"

class MenuPlayers : public Menu
{
public:
	void update(float _dt, GameStateData & _data) override;
	void draw(sf::RenderTexture & _dst) override;

	void onMenuLeft(uint _player) override;
	void onMenuRight(uint _player) override;
	void onMenuAction(uint _player, unsigned int _button) override;
};