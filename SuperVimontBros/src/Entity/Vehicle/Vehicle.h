#pragma once

#include "Entity/Entity.h"

struct Wheel
{
	Visual * visual;
	sf::Vector2f pos;
};

struct PassengerInfo
{
	Entity * entity;
	sf::Vector2f offset;
};

class Vehicle : public Entity
{
public:
	Vehicle(const sf::String & _name);
	~Vehicle();

	void init() override;
	void update(const float _dt) override;

	void onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;
	void onObjectCollision(objectIndex _objectIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;
	void onTileCollision(tileIndex _tileIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;

	void onVehicleCollision(Visual * _other, bool _fromOtherVehicle = false);

	Entity * getDriver() const;

	bool addPassenger(Entity * _entity);
	bool removePassenger(Entity * _entity);

	void addWheel(const sf::Vector2u _image, sf::Vector2f _position);

protected:
	void removeWheels();

protected:
	typedef Entity Super;

	std::vector<PassengerInfo> m_passengers;
	std::vector<Wheel> m_wheels;
	uint m_brokenWheels = 0;

	float m_baseSpeedForward = 1.5f;
	float m_baseSpeedBackward = 1.0f;
	float m_turboSpeedForward = 2.0f;
	float m_turboSpeedBackward = 1.25f;

	float m_maxSpeedForward = 0.5f;
	float m_maxSpeedBackward = 0.3f;

	float m_steering = 1.0f;

	sf::Clock m_addPassengerTimer;
};