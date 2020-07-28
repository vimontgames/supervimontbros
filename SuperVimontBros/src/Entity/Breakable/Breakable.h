#pragma once

#include "Entity/Entity.h"

enum class BreakableType : u8
{
	SandCastle = 0,

	Count
};

class Breakable : public Entity
{
public:
	Breakable(BreakableType _breakableType);
	void init() override;
	void onBeingCollided(Actor * _byActor, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;

private:
	BreakableType m_breakableType;
	bool m_broken = false;

	using Super = Entity;
};