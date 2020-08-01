#pragma once

#include "Entity/Entity.h"

enum class DogType : u8
{
	June = 0,
	Lucky,
	Praline,

	Count
};

class Dog : public Entity
{
public:
	Dog(DogType _dogType);
	~Dog();

	void init() override;
	void update(const float _dt) override;
	void onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;

	const char * getDogName(DogType _dogType) const;

private:
	using Super = Entity;

	DogType m_dogType;
	float m_speed = 0.03f;
	float m_wakeUpTime = 3.0f;
	float m_shitTime = 30.0f;
	bool m_walking = true;
	bool m_shitting = false;
	sf::Clock m_walkTimer;
	sf::Clock m_shitTimer;
};