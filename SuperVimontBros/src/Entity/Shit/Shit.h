#pragma once

#include "Entity/Entity.h"

class Player;

enum class ShitType
{
	Default = 0,
	Ball,
	Rugby,
	Electric,
	ZombieHead,
	ZombieHeadSkeleton,

	Count
};

class Shit : public Entity
{
public:
	Shit(ShitType _shitType = ShitType::Default);
	~Shit();

	void init() override;
	void update(const float _dt) override;

	void onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;

	ShitType getShitType() const;
	bool isBallShit() const;

private:
	using Super = Entity;
	ShitType m_shitType;
	uint m_lifeTime;
};

#ifdef ENABLE_INL
#include "Shit.inl"
#endif