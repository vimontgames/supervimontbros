#pragma once

#include "Entity/Entity.h"

enum class BallType
{
	FootballWhite = 0,
	FootballYellow,
	BeachBall,
	Rugby,
	RugbyWhite,
	Tennis,

	Count
};

class Ball : public Entity
{
public:
	Ball(BallType _ballType);
	~Ball();

	void init() override;
	void update(const float _dt) override;

	void onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;
	void onObjectCollision(objectIndex _objectIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;
	void onTileCollision(tileIndex _tileIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;

	static void setBallProperties(Entity * _entity, BallType _ballType);

	bool isFootBall() const;
	bool isRugbyBall() const;
	bool isTennisBall() const;

	bool playHitSound();

	BallType getBallType() const;

private:
	using Super = Entity;

	BallType m_ballType;
	sf::Clock m_hitSoundTimer;
};

#ifdef ENABLE_INL
#include "Ball.inl"
#endif