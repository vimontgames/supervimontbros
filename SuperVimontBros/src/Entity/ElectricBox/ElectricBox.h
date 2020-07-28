#pragma once

#include "Entity/Entity.h"

class Player;

class ElectricBox : public Entity
{
public:
	ElectricBox(bool _open);
	void init() override;

	bool isOpen() const { return m_open; }
	void open(Entity * _openedBy);
	void close();

	void onBeingCollided(Actor * _byActor, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;

private:
	bool m_open;

	using Super = Entity;
};