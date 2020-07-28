#pragma once

#include "Entity/Entity.h"

class Player;
enum class EnemyType : u8;

enum class SplashType
{
	Enemy = 0,
	Cinder,
	PlayerThrowUp,

	Count
};

class Splash : public Entity
{
public:
	Splash(SplashType _type, uint _subIndex);
	~Splash();

	void init() override;
	void update(const float _dt) override;

private:
	using Super = Entity;
	SplashType m_splashType;
	uint m_subIndex;
	uint m_lifeTime;
};