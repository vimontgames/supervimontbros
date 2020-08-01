#pragma once

#include "Entity/Entity.h"

enum class BonusType : u8
{
	Coca = 0,
	Water,
	Hotdog,
	Pizza,
	ChocolateEclair,
	Crunch,
	IceCream,
	Wine,

	BluePill,
	RedPill,
	GreenPill,

	Bone,
	Chicken,
	ChickenBone,
	PureeSausage,

	Apple,

	Count
};

enum class PlayerType : u8;

class Bonus : public Entity
{
public:
	Bonus(const sf::String & _name, BonusType _bonusType);
	~Bonus();

	void init() override;
	void respawn() override;
	void update(const float _dt) override;

	void fadeOutAndKill(uint _ms);

	void setRotten(bool _rotten);

	bool isRotten() const;
	bool isRottenForPlayer(PlayerType _playerType) const;
	bool isWine() const;
	bool isChocolate() const;

	BonusType	m_bonusType;
	bool		m_isRotten = false;
	bool		m_forAdultsOnly = false;
	bool		m_isForDogsOnly = false;
	bool		m_isSpawningOtherBonus = false;

	using Super = Entity;
};