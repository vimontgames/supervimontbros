#include "Precomp.h"
#include "Shit.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"
#include "Entity/Dog/Dog.h"
#include "Entity/Ball/Ball.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Tiles/LevelObject.h"

#ifndef ENABLE_INL
#include "Shit.inl"
#endif

using namespace sf;

//--------------------------------------------------------------------------
Shit::Shit(ShitType _shitType) :
	Entity("Shit", Game::get().m_objectTiles),
	m_shitType(_shitType)
{
	SuperVimontBros::get().m_shits.add(this);

	m_rotateWhenMoving = true;

	switch (m_shitType)
	{
		default:
			m_kickSpeed = 0.2f;
			m_bounce = 0.3f;
			m_lifeTime = 10000;
			break;

		case ShitType::Ball:
			Ball::setBallProperties(this, BallType::FootballWhite);
			m_lifeTime = 20000;
			break;

		case ShitType::Rugby:
			Ball::setBallProperties(this, BallType::Rugby);
			m_lifeTime = 20000;
			break;

		case ShitType::Electric:
			m_lifeTime = 20000;
			m_kickSpeed = 0.3f;
			m_bounce = 0.6f;
			break;

		case ShitType::PatrickHead:
		case ShitType::ZombieHeadSkeleton:
		case ShitType::ZombieHead:
			m_kickSpeed = 0.25f;
			m_bounce = 0.5f;
			m_lifeTime = 15000;
			break;

	}
}

//--------------------------------------------------------------------------
Shit::~Shit()
{
	SuperVimontBros::get().m_shits.remove(this);
}

//--------------------------------------------------------------------------
void Shit::init()
{
	Entity::init();

	AnimationSequence & idle = getAnimationSequence(Animation::Idle);
	
	switch (m_shitType)
	{
		default:
			assert(false);  

		case ShitType::Default:
			idle.addFrame(AnimFrame(m_tileSet->indexToCoords((uint)LevelObject::Shit)) );
			break; 

		case ShitType::Ball:
			idle.addFrame(AnimFrame(m_tileSet->indexToCoords((uint)LevelObject::ShitFootball)));
			break;

		case ShitType::Electric:
			idle.addFrame(AnimFrame(m_tileSet->indexToCoords((uint)LevelObject::ShitElectric), 1000));
			idle.addFrame(AnimFrame(m_tileSet->indexToCoords((uint)LevelObject::ShitElectric1), 100));
			break;

		case ShitType::Rugby:
			idle.addFrame(AnimFrame(m_tileSet->indexToCoords((uint)LevelObject::ShitRugby)));
			break;

		case ShitType::ZombieHead:
			idle.addFrame(AnimFrame(m_tileSet->indexToCoords((uint)LevelObject::ZombieHead)));
			break;

		case ShitType::ZombieHeadSkeleton:
			idle.addFrame(AnimFrame(m_tileSet->indexToCoords((uint)LevelObject::ZombieHeadSkeleton)));
			break;

		case ShitType::PatrickHead:
			idle.addFrame(AnimFrame(m_tileSet->indexToCoords((uint)LevelObject::PatrickHead)));
			break;			
	}

	addSoundFX(SoundFX::Hit, "SuperVimontBros/data/sound/Poc.wav");
}

//--------------------------------------------------------------------------
void Shit::onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	Shit * shit = dynamic_cast<Shit*>(_other);
	if (shit)
	{
		shit->m_velocity += m_velocity * shit->m_bounce * 0.5f;
		shit->setParent(this->getParent());
	}

	Enemy * enemy = dynamic_cast<Enemy*>(_other);
	if (enemy)
	{
		if (enemy->onShitHit(this))
			return;
	}

	Player * player = dynamic_cast<Player*>(_other);
	if (player)
	{
		player->onShitHit(this);
		return;
	}

	Dog * dog = dynamic_cast<Dog*>(_other);
	if (dog)
	{
		return;
	}

	Super::onActorCollision(_other, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
void Shit::update(const float _dt)
{
	const uint t = m_animTimer.getElapsedTime().asMilliseconds();

	if (t > m_lifeTime)
	{
		fadeOutAndKill(1000);
	}

	Entity::update(_dt);

	if ( t <= 1000)
	{
		uint fade = t * 255 / 1000;
		const Color & color = m_sprite.getColor();
		setColor(Color(color.r, color.g, color.b, fade));

		move(0.0f, 0.0f, -_dt * 5.0f / 1000.0f);
	}

	playAnimation(Animation::Idle, m_animLeft);
}