#pragma once

#include "MenuTitle.h"

class MenuCredits : public Menu
{
public:
	void update(float _dt, GameStateData & _data) override;
	void draw(sf::RenderTexture & _dst) override;

	void onEnterMenu() override;
	void onMenuAction(uint _player, unsigned int _button) override;

private:
	float m_timeStart = 0.0f;
};