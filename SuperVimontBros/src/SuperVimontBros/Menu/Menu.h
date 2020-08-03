#pragma once

namespace sf { class RenderTexture; }
class GameStateData;

class Menu
{
public:
	virtual void update(float _dt, GameStateData & _data) {};
	virtual void draw(sf::RenderTexture & _dst) {};

	virtual void onEnterMenu() {};

	virtual void onMenuUp(uint _player) {};
	virtual void onMenuDown(uint _player) {};
	virtual void onMenuLeft(uint _player) {};
	virtual void onMenuRight(uint _player) {};
	virtual void onMenuAction(uint _player, unsigned int _button) {};
};