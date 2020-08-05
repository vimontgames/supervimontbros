#include "Precomp.h"
#include "Bullet.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"
#include "Entity/Vehicle/Vehicle.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Tiles/LevelObject.h"

#ifndef ENABLE_INL
#include "Bullet.inl"
#endif

using namespace sf;

//--------------------------------------------------------------------------
Bullet::Bullet(BulletType _bulletType) :
	Entity("Bullet", Game::get().m_objectTiles),
	m_bulletType(_bulletType)
{
	SuperVimontBros::get().m_bullets.add(this);

	m_rotateWhenMoving = false;

	switch (m_bulletType)
	{
	default:
		m_kickSpeed = 0.02f;
		m_friction = 0.0f;
		m_bounce = 0.0f;
		m_lifeTime = 3000;
		break;
	}
}

//--------------------------------------------------------------------------
Bullet::~Bullet()
{
	SuperVimontBros::get().m_bullets.remove(this);
}

//--------------------------------------------------------------------------
void Bullet::init()
{
	Entity::init();

	AnimationSequence & idle = getAnimationSequence(Animation::Idle);

	switch (m_bulletType)
	{
		default:
			idle.addFrame(AnimFrame(m_tileSet->indexToCoords((uint)LevelObject::Bullet)));
			break;
	}

	//addSoundFX(SoundFX::Hit, "SuperVimontBros/data/sound/Poc.wav");
	setImage(m_tileSet->indexToCoords((uint)LevelObject::Bullet), m_faceLeft);
}

//--------------------------------------------------------------------------
void Bullet::onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	Vehicle * vehicle = dynamic_cast<Vehicle*>(_other);
	if (vehicle)
	{
		if (vehicle->onVehicleCollision(this, true))
			return;
	}

	Enemy * enemy = dynamic_cast<Enemy*>(_other);
	if (enemy && enemy != this->getParent())
	{
		if (enemy->onBulletHit(this))
			return;
	}
	
	Player * player = dynamic_cast<Player*>(_other);
	if (player)
	{
		player->onBulletHit(this);
		return;
	}
	//
	//Dog * dog = dynamic_cast<Dog*>(_other);
	//if (dog)
	//{
	//	return;
	//}

	//Super::onActorCollision(_other, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
void Bullet::onObjectCollision(objectIndex _objectIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	const uint t = m_animTimer.getElapsedTime().asMilliseconds();
	if (t > 0)
		fadeOutAndKill(100);
}

//--------------------------------------------------------------------------
void Bullet::onTileCollision(tileIndex _tileIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	const uint t = m_animTimer.getElapsedTime().asMilliseconds();
	if (t > 0)
		fadeOutAndKill(100);
}

//--------------------------------------------------------------------------
void Bullet::update(const float _dt)
{
	playAnimation(Animation::Idle, m_animLeft);

	const uint t = m_animTimer.getElapsedTime().asMilliseconds();
	
	if (t > m_lifeTime)
		fadeOutAndKill(100);
	
	Entity::update(_dt);

	//
	//if (t <= 1000)
	//{
	//	uint fade = t * 255 / 1000;
	//	const Color & color = m_sprite.getColor();
	//	setColor(Color(color.r, color.g, color.b, fade));
	//
	//	move(0.0f, 0.0f, -_dt * 5.0f / 1000.0f);
	//}	
}