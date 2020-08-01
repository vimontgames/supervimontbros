#include "Precomp.h"
#include "Bonus.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Tiles/LevelObject.h"
#include "Entity/Player/Player.h"

using namespace sf;

//--------------------------------------------------------------------------
Bonus::Bonus(const String & _name, BonusType _bonusType) :
	Entity(_name, Game::get().m_objectTiles),
	m_bonusType(_bonusType)
{
	SuperVimontBros::get().m_bonuses.add(this);	
	m_isColliderForOtherActors = false;

	m_respawn = true;
	m_respawnTime = 30.0f + (float)(rand() % 100);
}

//--------------------------------------------------------------------------
Bonus::~Bonus()
{
	SuperVimontBros::get().m_bonuses.remove(this);
}

//--------------------------------------------------------------------------
void Bonus::init()
{
	Entity::init();

	const auto coords = m_coords;

	AnimationSequence & idle = getAnimationSequence(Animation::Idle);
						idle.addFrame(AnimFrame(coords));

	bool rotten = false;

	switch (m_bonusType)
	{
		case BonusType::Apple:
			m_hp = 4;
			rotten = (rand() & 3) == 0;
			break;

		case BonusType::Wine:
			m_hp = 4;
			break;

		case BonusType::Crunch:
			m_hp = 2;
			rotten = (rand() & 15) == 0;
			break;

		case BonusType::IceCream:
			m_hp = 3;
			rotten = (rand() & 7) == 0;
			break;

		case BonusType::ChocolateEclair:
			m_hp = 1;
			rotten = (rand() & 7) == 0;
			break;

		case BonusType::Pizza:
			m_hp = 4;
			rotten = (rand() & 15) == 0;
			break;

		case BonusType::Coca:
			m_hp = 1;
			rotten = (rand() & 15) == 0;
			break;

		case BonusType::Water:
			m_hp = 2;
			rotten = (rand() & 31) == 0;
			break;

		case BonusType::Hotdog:
			m_hp = 1;
			rotten = (rand() % 3) == 0;
			break;

		case BonusType::BluePill:
			m_hp = 1;
			break;

		case BonusType::RedPill:
			m_hp = 1;
			break;

		case BonusType::GreenPill:
			m_respawn = true;
			m_hp = 1;
			break;

		case BonusType::Bone:
			m_respawn = true;
			m_isForDogsOnly = true;
			m_hp = 4;
			break;

		case BonusType::Chicken:
			m_respawn = false;
			m_hp = 4;
			break;

		case BonusType::ChickenBone:
			m_respawn = false;
			m_isForDogsOnly = true;
			m_hp = 2;
			break;

		case BonusType::PureeSausage:
			m_hp = 4;
			break;
	}

	setRotten(rotten);
	playAnimation(Animation::Idle);
}

//--------------------------------------------------------------------------
void Bonus::setRotten(bool _rotten)
{
	if (_rotten)
	{
		m_isRotten = true;
		setColor({ 187,255,128,255 });
	}
	else
	{
		m_isRotten = false;
		setColor({ 255,255,255,255 });
	}
}

//--------------------------------------------------------------------------
bool Bonus::isWine() const
{
	switch (m_bonusType)
	{
		case BonusType::Wine:
			return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Bonus::isChocolate() const
{
	switch (m_bonusType)
	{
	case BonusType::Crunch:
	case BonusType::ChocolateEclair:
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Bonus::isRottenForPlayer(PlayerType _playerType) const
{
	if (isWine() && !(PlayerTypeInfo::get(_playerType).flags & PlayerTypeInfo::Flags::CanDrinkWine))
		return true;
	else if (isChocolate() && !(PlayerTypeInfo::get(_playerType).flags & PlayerTypeInfo::Flags::IsADog))
		return true;
	return m_isRotten;
}

//--------------------------------------------------------------------------
bool Bonus::isRotten() const
{
	return m_isRotten;
}

//--------------------------------------------------------------------------
void Bonus::update(const float _dt)
{
	Entity::update(_dt);	
}

//--------------------------------------------------------------------------
void Bonus::fadeOutAndKill(uint _ms)
{
	switch (m_bonusType)
	{
		case BonusType::Chicken:
		{
			Bonus * bonus = new Bonus("ChickenBones", BonusType::ChickenBone);

			bonus->setImage(m_tileSet->indexToCoords((uint)LevelObject::ChickenBone));
			bonus->init();
			bonus->setPosition(getPosition());
			bonus->m_isRotten = m_isRotten;
			bonus->setColor(m_sprite.getColor());

			Game::get().registerVisual(bonus);
			bonus->release();
		}
		break;
	}

	Super::fadeOutAndKill(_ms);
}

//--------------------------------------------------------------------------
void Bonus::respawn()
{
	switch (m_bonusType)
	{
		case BonusType::ChickenBone:
		{
			m_bonusType = BonusType::Chicken;
			setImage(m_tileSet->indexToCoords((uint)LevelObject::Chicken));
		}
		break;
	}

	Super::respawn();
}