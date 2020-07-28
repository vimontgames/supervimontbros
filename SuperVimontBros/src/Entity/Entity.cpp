#include "Precomp.h"
#include "Entity.h"
#include "Viewport/Viewport.h"
#include "Level/Level.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"
#include "Ball/Ball.h"
#include "Shit/Shit.h"

#ifndef ENABLE_INL
#include "Entity.inl"
#endif

using namespace sf;

EntityGUID Entity::s_nextGUID = 0;

//--------------------------------------------------------------------------
Entity::Entity(const sf::String & _name, const Tiles & _tileSet) :
	Actor(_name, _tileSet)
{
	m_GUID = s_nextGUID++;
}

//--------------------------------------------------------------------------
Entity::~Entity()
{

}

//--------------------------------------------------------------------------
void Entity::onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	if ( (getParent() == _other && !m_isCollidingWithParent) || (_other->getParent() == this && !_other->m_isCollidingWithParent) )
		return;

	Super::onActorCollision(_other, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
void Entity::updateAABB()
{
	Super::updateAABB();

	//if (m_pickedEntity)
	//{
	//	const auto tileInfo = m_pickedEntity->getTileInfo();
	//
	//	Vector2f vMin = 
	//	{
	//		min(m_collisionAABB.m_pos.x - m_collisionAABB.m_half.x, m_pickedEntity->m_collisionAABB.m_pos.x - m_pickedEntity->m_collisionAABB.m_half.x + tileInfo.m_hotSpot.x),
	//		min(m_collisionAABB.m_pos.y - m_collisionAABB.m_half.y, m_pickedEntity->m_collisionAABB.m_pos.y - m_pickedEntity->m_collisionAABB.m_half.y + tileInfo.m_hotSpot.y)
	//	};
	//
	//	Vector2f vMax =
	//	{
	//		max(m_collisionAABB.m_pos.x + m_collisionAABB.m_half.x, m_pickedEntity->m_collisionAABB.m_pos.x + m_pickedEntity->m_collisionAABB.m_half.x + tileInfo.m_hotSpot.x),
	//		max(m_collisionAABB.m_pos.y + m_collisionAABB.m_half.y, m_pickedEntity->m_collisionAABB.m_pos.y + m_pickedEntity->m_collisionAABB.m_half.y + tileInfo.m_hotSpot.y)
	//	};
	//
	//	m_collisionAABB = AABB( (vMin + vMax) * 0.5f, (vMax - vMin)*0.5f );
	//}
}

//--------------------------------------------------------------------------
std::vector<Entity*> Entity::findEntitiesInRadius(const float _lookupDist)
{
	std::vector<Entity*> closeEntities;

	const auto & list = Game::get().m_actors;
	const uint actorCount = list.size();

	for (uint b = 0; b < actorCount; ++b)
	{
		Entity * entity = (Entity*)list[b];

		if (entity != this && dynamic_cast<Player*>(entity) == nullptr && dynamic_cast<Enemy*>(entity) == nullptr)
		{
			Vector2f delta = entity->getPosition() - getPosition();
			float dist = length(delta);

			if (dist < _lookupDist)
			{
				closeEntities.push_back(entity);
			}
		}
	}

	return closeEntities;
}

//--------------------------------------------------------------------------
AnimationSequence & Entity::getAnimationSequence(Animation _animation)
{
	return Super::getAnimationSequence((AnimIndex)_animation);
}

//--------------------------------------------------------------------------
bool Entity::playAnimation(Animation _animation, bool _left, bool _restart)
{
	return Super::playAnim((AnimIndex)_animation, _left, _restart);
}

//--------------------------------------------------------------------------
bool Entity::isAnimationPlaying(Animation _animation) const
{
	return _animation == (Animation)m_animIndex;
}

//--------------------------------------------------------------------------
bool Entity::addSoundFX(SoundFX _soundFX, const char * _fullpath)
{
	return Super::addSound((SoundIndex)_soundFX, _fullpath);
}

//--------------------------------------------------------------------------
bool Entity::addCustomSoundFX(SoundFX _soundFX, const char * _path, const char * _filename)
{
	return Super::addCustomSound((SoundIndex)_soundFX, _path, _filename);
}

//--------------------------------------------------------------------------
bool Entity::playSound(SoundFX _soundFX, int _volume, bool _loop)
{
	return Super::playSound((SoundIndex)_soundFX, _volume, _loop);
}

//--------------------------------------------------------------------------
bool Entity::clampPositionToLevel(const Level & _level)
{
	//const Vector2u levelSize = _level.count();
	//
	//const auto & tileInfo = getTileInfo();
	//const Vector2f origin = Vector2f(tileInfo.m_origin);
	//const Vector2f & pos = getPosition() - origin;
	//Vector2f border = { (float)tileInfo.m_box.width / 2, (float)tileInfo.m_box.height / 2 };
	//
	//const uint size = 32;
	//
	//if (pos.x < border.x)
	//{
	//	setPosition(border.x, pos.y);
	//}
	//else if (pos.x > levelSize.x * size - border.x)
	//{
	//	setPosition(levelSize.x * size - border.x, pos.y);
	//}
	//
	//if (pos.y < border.y)
	//{
	//	setPosition(pos.x, border.y);
	//}
	//else if (pos.y > levelSize.y * size - border.y)
	//{
	//	setPosition(pos.x, levelSize.y * size - border.y);
	//}
	//
	//return pos != getPosition();

	return false;
}

//--------------------------------------------------------------------------
bool Entity::clampPositionToViewport(const Viewport & _viewport)
{
	//const View & view = _viewport.getView();
	//
	//const auto & tileInfo = getTileInfo();
	//Vector2f border = { (float)tileInfo.m_box.width / 2, (float)tileInfo.m_box.height / 2 };
	//
	//const float rightScreen = view.getCenter().x + view.getSize().x / 2 - border.x;
	//const float leftScreen = view.getCenter().x - view.getSize().x / 2 + border.x;
	//const float topScreen = view.getCenter().y + view.getSize().y / 2 - border.y;
	//const float bottomScreen = view.getCenter().y - view.getSize().y / 2 + border.y;
	//
	//const Vector2f pos = getPosition();
	//
	//if (pos.x >= rightScreen)
	//{
	//	setPosition({ rightScreen, pos.y });
	//}
	//else if (pos.x < leftScreen)
	//{
	//	setPosition(leftScreen, pos.y);
	//}
	//
	//if (pos.y >= topScreen)
	//{
	//	setPosition(pos.x, topScreen);
	//}
	//else if (pos.y < bottomScreen)
	//{
	//	setPosition(pos.x, bottomScreen);
	//}
	//
	//return pos != getPosition();
	return false;
}

//--------------------------------------------------------------------------
bool Entity::updatePickedEntityPos()
{
	if (m_pickedEntity)
	{
		float x = getTileInfo().m_hotSpot.x;

		if (!m_hasAnimLeft && m_faceLeft)
		{
			x *= -1;
		}

		m_pickedEntity->setPosition(getPosition() + Vector2f(x, 0));
		m_pickedEntity->m_velocity = { 0,0, };
		m_pickedEntity->m_jump = 0;
		m_pickedEntity->m_height = (float)-getTileInfo().m_hotSpot.y;

		if (Ball * ball = dynamic_cast<Ball*>(m_pickedEntity))
		{
			if (ball->isRugbyBall())
			{
				m_pickedEntity->m_angle = -90.0f;
			}
			else
			{
				m_pickedEntity->m_angle = 0.0f;
			}
		}
		return true;
	}

	return false;
}


//--------------------------------------------------------------------------
bool Entity::dropPickedEntity()
{
	if (m_pickedEntity)
	{
		// TODO: return false if we cannot drop it here
		m_pickedEntity->m_jump = 0.0f;
		m_pickedEntity->m_height = (float)-getTileInfo().m_hotSpot.y;
		m_pickedEntity->setPicked(false);
		m_pickedEntity->m_isColliderForOtherActors = true;
		m_pickedEntity = nullptr;

		return true;
	}

	return false;
}

