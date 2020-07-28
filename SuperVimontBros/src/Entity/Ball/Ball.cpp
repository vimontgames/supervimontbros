#include "Precomp.h"
#include "Ball.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Sound/SoundBank.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"

#ifndef ENABLE_INL
#include "Ball.inl"
#endif

using namespace sf;

//--------------------------------------------------------------------------
Ball::Ball(BallType _ballType) :
	Entity("Ball", Game::get().m_objectTiles),
	m_ballType(_ballType)
{
	SuperVimontBros::get().m_balls.add(this);

	m_rotateWhenMoving = true;
	setIsPickable(true);
	setBallProperties(this, m_ballType);	
}

//--------------------------------------------------------------------------
void Ball::setBallProperties(Entity * _entity, BallType _ballType)
{
	switch (_ballType)
	{
		default:
		case BallType::FootballWhite:
			_entity->m_kickSpeed = 0.3f;
			_entity->m_bounce = 0.75f;
			_entity->m_friction = 1.0f / 1750.0f;
			break;

		case BallType::FootballYellow:
			_entity->m_kickSpeed = 0.3f;
			_entity->m_bounce = 0.75f;
			_entity->m_friction = 1.0f / 1750.0f;
			break;

		case BallType::BeachBall:
			_entity->m_kickSpeed = 0.2f;
			_entity->m_bounce = 0.90f;
			_entity->m_friction = 1.0f / 3000.0f;
			break;

		case BallType::Rugby:
		case BallType::RugbyWhite:
			_entity->m_kickSpeed = 0.4f;
			_entity->m_bounce = 0.85f;
			_entity->m_friction = 1.0f / 1250.0f;
			break;

		case BallType::Tennis:
			_entity->m_kickSpeed = 0.25f;
			_entity->m_bounce = 0.9f;
			_entity->m_friction = 1.0f / 1000.0f;
			break;
	}
}

//--------------------------------------------------------------------------
Ball::~Ball()
{
	SuperVimontBros::get().m_balls.remove(this);
}

//--------------------------------------------------------------------------
void Ball::init()
{
	Entity::init();

	AnimationSequence & idleRight = getAnimationSequence(Animation::Idle);
						idleRight.addFrame(AnimFrame({ (uint)m_ballType,4 }));

	addSoundFX(SoundFX::Hit, "SuperVimontBros/data/sound/Poc.wav");
}

//--------------------------------------------------------------------------
void Ball::onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	Enemy * enemy = dynamic_cast<Enemy*>(_other);
	if (enemy)
	{
		if (enemy->onHitBall(this))
			return;
	}

	Ball * ball = dynamic_cast<Ball*>(_other);
	if (ball)
	{
		ball->m_velocity += m_velocity * ball->m_bounce * 0.5f;
		ball->setParent(getParent());
	}

	playHitSound();

	Super::onActorCollision(_other, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
void Ball::onObjectCollision(objectIndex _objectIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	playHitSound();
	Super::onObjectCollision(_objectIndex, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
void Ball::onTileCollision(tileIndex _tileIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	playHitSound();
	Super::onTileCollision(_tileIndex, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
bool Ball::playHitSound()
{
	if (m_hitSoundTimer.getElapsedTime().asMilliseconds() > 250)
	{
		playSound(SoundFX::Hit);
		m_hitSoundTimer.restart();
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
void Ball::update(const float _dt)
{
	Entity::update(_dt);

	float norm = length(m_velocity);
	if (norm >= 0.001f && getParent())
	{
		addDebugText("n = %.2f\nz = %.2f\n", norm, m_height);
	}
	else 
	{
		if (getParent() && !isPicked())
		{
			auto & game = SuperVimontBros::get();
			bool anyPlayerClose = false;
			for (auto * player : game.m_players)
			{
				const auto dist = length(this->getPosition() - player->getPosition());

				if (dist < 16)
				{
					anyPlayerClose = true;
					break;
				}
			}

			if (!anyPlayerClose)
			{
				setParent(nullptr);
			}
		}
	}

	if (getParent())
	{
		//addDebugText("%.2f\n(%s)", m_height, m_parent->name().toAnsiString().c_str());
	}

	playAnimation(Animation::Idle);
}