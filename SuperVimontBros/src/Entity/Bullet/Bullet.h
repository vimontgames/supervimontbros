#pragma once

#include "Entity/Entity.h"

class Player;

enum class BulletType
{
	Default = 0,
	
	Count
};

class Bullet : public Entity
{
public:
	Bullet(BulletType _shitType = BulletType::Default);
	~Bullet();

	void init() override;
	void update(const float _dt) override;

	void onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;
	void onObjectCollision(objectIndex _objectIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;
	void onTileCollision(tileIndex _tileIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;

	Bullet getBulletType() const;

private:
	using Super = Entity;
	BulletType m_bulletType;
	uint m_lifeTime;
};

#ifdef ENABLE_INL
#include "Bullet.inl"
#endif