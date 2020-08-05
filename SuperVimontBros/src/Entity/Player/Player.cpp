#include "Precomp.h"
#include "Player.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Entity/Shit/Shit.h"
#include "Entity/Ball/Ball.h"
#include "Entity/Goal/Goal.h"
#include "Entity/Dog/Dog.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Vehicle/Vehicle.h"
#include "Entity/Bonus/Bonus.h"
#include "Entity/ElectricBox/ElectricBox.h"
#include "Entity/Splash/Splash.h"
#include "Physics/AABB.h"
#include "Sound/SoundBank.h"
#include "Viewport/Viewport.h"
#include "PostProcess/PostProcess.h"
#include "Tiles/SpriteModel.h"

#ifndef ENABLE_INL
#include "Player.inl"
#endif

using namespace sf;

static const int timeBeforeDamage = 0;
static const int damageDuration = 500;
static const int timeBeforeNewDamage = 500;

//--------------------------------------------------------------------------
const PlayerTypeInfo & PlayerTypeInfo::get(PlayerType _playerType)
{
	static PlayerTypeInfo playerTypeInfo[] =
	{
		PlayerTypeInfo("ROMEO",	   SPRITE_LINE(SpriteModel::Romeo),	   Color(255,255,230),   AnimLeft      | CanCarry | ShitFootBall,				2, 1, 3, 4),
		PlayerTypeInfo("PABLO",	   SPRITE_LINE(SpriteModel::Pablo),	   Color(250, 250, 250), StrongStomach | CanCarry | ShitRugby,				    1, 3, 2, 4),
		PlayerTypeInfo("BENUALDO", SPRITE_LINE(SpriteModel::Benualdo), Color(128,96,64),     AnimLeft      | CanCarry | CanDrinkWine,				1, 0, 3, 2),
		PlayerTypeInfo("MARIE",	   SPRITE_LINE(SpriteModel::Marie),	   defaultShitColor,     AtomicFart    | CanCarry | CanDrinkWine,				0, 3, 4, 1),
		PlayerTypeInfo("YVAN",	   SPRITE_LINE(SpriteModel::Yvan),	   defaultShitColor,     StrongStomach | CanCarry | CanDrinkWine | ShitRugby,   0, 3, 1, 5),
		PlayerTypeInfo("JUNE",	   SPRITE_LINE(SpriteModel::June),	   defaultShitColor,     IsADog),
		PlayerTypeInfo("LUCKY",	   SPRITE_LINE(SpriteModel::Lucky),	   defaultShitColor,     IsADog),
		PlayerTypeInfo("PRALINE",  SPRITE_LINE(SpriteModel::Praline),  Color(230,255,230),   IsADog | AtomicFart),
        PlayerTypeInfo("BENJAMIN", SPRITE_LINE(SpriteModel::Benjamin), Color(255,255,255),   CanCarry | CanDrinkWine | ShitElectric,				8, 4, 0, 6),
	};
	static_assert(COUNT_OF(playerTypeInfo) == (uint)PlayerType::Count);

	assert((uint)_playerType < (uint)PlayerType::Count);
	return playerTypeInfo[(uint)_playerType];
}

//--------------------------------------------------------------------------
Player::Player(ControllerID _controllerID, PlayerType _playerType, u8 _playerTypeIndex) :
	Entity(PlayerTypeInfo::get(_playerType).name, Game::get().m_spritesTile),
	m_controllerID(_controllerID),
	m_playerType(_playerType),
	m_playerTypeIndex(_playerTypeIndex)
{
	if (PlayerTypeInfo::get(_playerType).flags & PlayerTypeInfo::Flags::AnimLeft)
	{
		m_hasAnimLeft = true;
	}

	deactivate();
}

//--------------------------------------------------------------------------
void Player::init()
{
	Entity::init();

	m_hp = 12;
	m_zombieKilled = 0;
	
	for (uint i = 0; i < (uint)GoalType::Count; ++i)
	{
		m_goalScored[i] = 0;
	}

	setupPlayerAnimations(this, m_playerType);

	addCustomSoundFX(SoundFX::Prout, "SuperVimontBros/data/sound", "prout.wav");
	addCustomSoundFX(SoundFX::CelebrateGoal, "SuperVimontBros/data/sound", "goal.wav");

	if (!addCustomSoundFX(SoundFX::CelebrateTry, "SuperVimontBros/data/sound", "goal2.wav"))
		addCustomSoundFX(SoundFX::CelebrateTry, "SuperVimontBros/data/sound", "goal.wav");

	if (!addCustomSoundFX(SoundFX::CelebrateDrop, "SuperVimontBros/data/sound", "drop.wav"))
		addCustomSoundFX(SoundFX::CelebrateTry, "SuperVimontBros/data/sound", "goal.wav");

	addCustomSoundFX(SoundFX::Damage, "SuperVimontBros/data/sound", "aie.wav");
	addCustomSoundFX(SoundFX::Miam, "SuperVimontBros/data/sound", "miam.wav");

	if (!addCustomSoundFX(SoundFX::MiamPizza, "SuperVimontBros/data/sound", "pizza.wav"))
		addCustomSoundFX(SoundFX::MiamPizza, "SuperVimontBros/data/sound", "miam.wav");

	if (!addCustomSoundFX(SoundFX::MiamPuree, "SuperVimontBros/data/sound", "puree.wav"))
		addCustomSoundFX(SoundFX::MiamPuree, "SuperVimontBros/data/sound", "miam.wav");

	addCustomSoundFX(SoundFX::Beurk, "SuperVimontBros/data/sound", "beurk.wav");

	m_viewport = &Game::get().getDefaultViewport(); // default
}

//--------------------------------------------------------------------------
void Player::setViewport(Viewport & _viewport)
{
	m_viewport = &_viewport;
}

//--------------------------------------------------------------------------
void Player::activate()
{
	setColor({ 255, 255, 255, 255 });
	m_isActive = true;
	m_isColliderForOtherActors = true;
}

//--------------------------------------------------------------------------
void Player::deactivate()
{
	setColor({ 255, 255, 255, 128 });
	m_isActive = false;
	m_isColliderForOtherActors = false;
}

//--------------------------------------------------------------------------
uint Player::getScore() const
{
	return m_score;
}

//--------------------------------------------------------------------------
uint Player::addScore(i16 _points)
{
	m_score = max(0, (int)m_score + _points);
	return m_score;
}
//--------------------------------------------------------------------------
uint Player::getZombieKilled() const
{
	return m_zombieKilled;
}

//--------------------------------------------------------------------------
uint Player::addZombieKilled(i16 _count)
{
	m_zombieKilled += _count;
	return m_zombieKilled;
}

//--------------------------------------------------------------------------
uint Player::getGoalScored(GoalType _goalType) const
{
	return m_goalScored[(uint)_goalType];
}

//--------------------------------------------------------------------------
uint Player::addGoalScored(const Goal * _goal, i16 _points, float _distance, GoalType _goalType, GoalSide _goalSide)
{
	SuperVimontBros & game = SuperVimontBros::get();

	for (u32 i = 0; i < game.m_enemies.size(); ++i)
	{
		Enemy * zombie = game.m_enemies[i];

		if (zombie->getInfo().flags & EnemyTypeInfo::Flags::GoalKeeper)
			continue;

		if (_goalType == GoalType::Football || (_goalType == GoalType::Rugby && _points == 5))
		{
			if (_goal->getGoalSurface().intersects(zombie->m_collisionAABB))
			{
				zombie->kill(this);
			}
		}
		else if (_goalType == GoalType::Rugby && _points == 3)
		{
			if (_goal->getHalfField().intersects(zombie->m_collisionAABB))
			{
				zombie->kill(this);
			}
		}
	}

	auto & scored = m_goalScored[(uint)_goalType];
	scored += _points;
	
	switch (_goalType)
	{
		case GoalType::Football:
			playSound(SoundFX::CelebrateGoal);
			break;

		case GoalType::Rugby:
			if (_points == 5)
			{
				playSound(SoundFX::CelebrateTry);
			}
			else
			{
				playSound(SoundFX::CelebrateDrop);
			}
			break;
	}
	

	game.onGoalScored(this, _points, _distance, _goalType, _goalSide);

	return scored;
}

//--------------------------------------------------------------------------
void Player::celebrate()
{
	m_celebrate = true;
}

//--------------------------------------------------------------------------
void Player::setupPlayerAnimations(Entity * _entity, PlayerType _playerType)
{
	uint playerLine = PlayerTypeInfo::get(_playerType).line;

	Player * player = dynamic_cast<Player*>(_entity);

	// Reset Anim
	_entity->m_animSequences.clear();
	_entity->m_animTimer.restart();
	_entity->m_animIndex = invalidAnimIndex;

	AnimationSequence & icon = _entity->getAnimationSequence(Animation::Icon);
					    icon.addFrame(AnimFrame({ 15,playerLine }));

	AnimationSequence & idle = _entity->getAnimationSequence(Animation::Idle);
						idle.addFrame(AnimFrame({ 0,playerLine}));

	AnimationSequence & walk = _entity->getAnimationSequence(Animation::Walk);
						walk.addFrame(AnimFrame({ 1,playerLine }, 200));
						walk.addFrame(AnimFrame({ 3,playerLine }, 200));
						walk.addFrame(AnimFrame({ 1,playerLine }, 200));
						walk.addFrame(AnimFrame({ 2,playerLine }, 200));
						walk.addFrame(AnimFrame({ 4,playerLine }, 200));
						walk.addFrame(AnimFrame({ 2,playerLine }, 200));

	AnimationSequence & run = _entity->getAnimationSequence(Animation::Run);
						run.addFrame(AnimFrame({ 1,playerLine }, 100));
						run.addFrame(AnimFrame({ 3,playerLine }, 100));
						run.addFrame(AnimFrame({ 1,playerLine }, 100));
						run.addFrame(AnimFrame({ 2,playerLine }, 100));
						run.addFrame(AnimFrame({ 4,playerLine }, 100));
						run.addFrame(AnimFrame({ 2,playerLine }, 100));

	AnimationSequence & punch = _entity->getAnimationSequence(Animation::Punch);
						punch.addFrame(AnimFrame({ 5,playerLine }, 500));

	AnimationSequence & prepareKick = _entity->getAnimationSequence(Animation::PrepareKick);
						prepareKick.addFrame(AnimFrame({ 6,playerLine }, 250));

	AnimationSequence & kick = _entity->getAnimationSequence(Animation::Kick);
						kick.addFrame(AnimFrame({ 7,playerLine }, 250));

	AnimationSequence & celebrate = _entity->getAnimationSequence(Animation::CelebrateGoal);
						celebrate.addFrame(AnimFrame({ 9, playerLine }, 250));
						celebrate.addFrame(AnimFrame({ 10, playerLine }, 250));

	AnimationSequence & die = _entity->getAnimationSequence(Animation::Die);
						die.addFrame(AnimFrame({ 11, playerLine }, 1000));

	AnimationSequence & electricity = _entity->getAnimationSequence(Animation::Electricity);
	for (uint i = 0; i < 5; ++i)
	{
		electricity.addFrame(AnimFrame({ 8, playerLine }, 50));
		electricity.addFrame(AnimFrame({ 11, playerLine }, 50));
	}

	AnimationSequence & throwUp = _entity->getAnimationSequence(Animation::ThrowUp);
						throwUp.addFrame(AnimFrame({ 20, playerLine }, 250));
						throwUp.addFrame(AnimFrame({ 21, playerLine }, 250));

	uint shitWarmupTime = 1000.0f;
	if (PlayerTypeInfo::get(_playerType).flags & PlayerTypeInfo::Flags::AtomicFart)
	{
		shitWarmupTime = 2500.0f;
	}
	else
	{
		shitWarmupTime = 1000.0f;
	}

	if (player)
	{
		player->m_shitWarmupTime = shitWarmupTime;
	}

	AnimationSequence & shit = _entity->getAnimationSequence(Animation::Shit);
						shit.addFrame(AnimFrame({ 13,playerLine }, shitWarmupTime));
						shit.addFrame(AnimFrame({ 14,playerLine }, shitWarmupTime));
						
	AnimationSequence & idleCarry = _entity->getAnimationSequence(Animation::IdleCarry);
						idleCarry.addFrame(AnimFrame({ 12, playerLine }, 200));

	AnimationSequence & carry = _entity->getAnimationSequence(Animation::Carry);
						carry.addFrame(AnimFrame({ 16, playerLine }, 200));
						carry.addFrame(AnimFrame({ 17, playerLine }, 200));
						carry.addFrame(AnimFrame({ 18, playerLine }, 200));
						carry.addFrame(AnimFrame({ 19, playerLine }, 200));
};

//--------------------------------------------------------------------------
void Player::onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	//Ball * ball = dynamic_cast<Ball*>(_other);
	//if (ball)
	//{
	//	//if (!ball->isSoundPlaying((SoundIndex)SoundFX::Damage) && (m_dir.x != 0 && m_dir.y != 0))
	//	{
	//		ball->m_velocity += 0.05f * m_dir / norm2D(m_dir) ;
	//		//ball->playSound(SoundFX::Damage);
	//		ball->m_parent = this;
	//		return;
	//	}
	//}

	Dog * dog = dynamic_cast<Dog*>(_other);
	if (dog)
	{
		if (dog->getParent() == nullptr)
		{
			dog->setParent(this);
			dog->playSound(SoundFX::CelebrateGoal, 100);
			addScore(250);
		}
	}

	Shit * shit = dynamic_cast<Shit*>(_other);
	if (shit)
	{
		onShitHit(shit);
		return;
	}

	Enemy * enemy = dynamic_cast<Enemy*>(_other);
	if (enemy)
	{
		onEnemyHit(enemy);
	}

	Super::onActorCollision(_other, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
void Player::onEnemyHit(Enemy * _enemy)
{
	if (!isInVehicle() && canTakeDamage())
	{
		if (_enemy->setAttacking())
		{
			//debugPrint("Player %s is attacked by %s\n", name().toAnsiString().c_str(), _enemy->name().toAnsiString().c_str());
			m_timeSinceLastHit.restart();
			m_isAttacked = true;
			m_damageTaken = false;
			m_damagePending = -1;
			m_damageType = PlayerDamageType::Zombie;
		}
	}
}

//--------------------------------------------------------------------------
bool Player::onElectricityHit(Entity * _byEntity)
{
	if (!isInVehicle() && canTakeDamage())
	{
		//debugPrint("Player %s is attacked by %s\n", name().toAnsiString().c_str(), _enemy->name().toAnsiString().c_str());
		m_timeSinceLastHit.restart();
		m_isAttacked = true;
		m_damageTaken = false;
		m_damagePending = -1;
		m_damageType = PlayerDamageType::Electricity;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Player::AddRemoveHP(int _hp)
{
	auto oldHp = m_hp;
	m_hp = clamp((int)m_hp + _hp, 0, (int)s_maxHp);
	return m_hp != oldHp;
}

//--------------------------------------------------------------------------
bool Player::onBonusHit(Bonus * _bonus, bool _share)
{
	const bool isDog = asBool(PlayerTypeInfo::get(m_playerType).flags & PlayerTypeInfo::Flags::IsADog);

	if (_bonus->m_isForDogsOnly && !isDog)
	{
		return false;
	}

	if (!_bonus->m_visible || _bonus->m_sprite.getColor().a < 255)
		return false;

	bool justHealed = false;

	switch (_bonus->m_bonusType)
	{
		case BonusType::RedPill:
		case BonusType::BluePill:
			return false;

		case BonusType::GreenPill:
			if (m_isRotten && _bonus->m_hp != 0)
			{
				setRotten(false);
				justHealed = true;
			}
			else
			{
				return false;
			}
		break;
	}

	if (_bonus->isRottenForPlayer(m_playerType) && _bonus->m_hp != 0)
	{
		m_damagePending = -(int)_bonus->m_hp / 2;
		m_damageType = PlayerDamageType::Rotten;

		if (!_share)
		{
			_bonus->m_hp = 0;
			_bonus->fadeOutAndKill(500); 

			setRotten(true);
		}
	}
	else if (AddRemoveHP(_bonus->m_hp) || justHealed)
	{
		if (!_share)
		{
			_bonus->m_hp = 0;
		}

		_bonus->fadeOutAndKill(500);

		addScore(_bonus->m_hp*10);

		switch (_bonus->m_bonusType)
		{
			default:
				playSound(SoundFX::Miam);
				break;

			case BonusType::Pizza:
				playSound(SoundFX::MiamPizza);
				break;

			case BonusType::PureeSausage:
				playSound(SoundFX::MiamPuree);
				break;
		}

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Player::setRotten(bool _isRotten)
{
	if (_isRotten)
	{
		if (!m_isRotten)
		{
			m_isRotten = true;
			m_isAttacked = true;
			m_damageTaken = false;
			m_timeSinceLastHit.restart();
			m_rottenTimer.restart();
		}
	}
	else
	{
		if (m_isRotten)
		{
			m_isRotten = false;
		}
	}

	return m_isRotten;
}

//--------------------------------------------------------------------------
void Player::onEnterVehicle(Vehicle * _vehicle)
{
	if (_vehicle->addPassenger(this))
	{
		m_visible = false;
		m_vehicle = _vehicle;
		setParent(m_vehicle);
	}
}

//--------------------------------------------------------------------------
void Player::onShitHit(Shit * _shit)
{
	if (_shit->getShitType() == ShitType::Electric && _shit->getParent() != this)
	{
		onElectricityHit(_shit);
		_shit->playSound(SoundFX::Damage, 50);
	}
}

//--------------------------------------------------------------------------
bool Player::canTakeDamage() const
{
	return m_timeSinceLastHit.getElapsedTime().asMilliseconds() > damageDuration + timeBeforeNewDamage;
}

//--------------------------------------------------------------------------
bool Player::onBulletHit(Bullet * _bullet)
{
	if (!isInVehicle() && canTakeDamage())
	{
		m_timeSinceLastHit.restart();
		m_isAttacked = true;
		m_damageTaken = false;
		m_damagePending = -1;
		m_damageType = PlayerDamageType::Bullet;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
void Player::onExitVehicle()
{
	sf::Vector2f move = { 0.0f, 0.0f };

	auto result = SuperVimontBros::get().tryMove(this, move);
	if (result.canMoveX && result.canMoveY)
	{
		if (m_vehicle->removePassenger(this))
		{
			m_vehicle = nullptr;
			m_visible = true;
			setParent(nullptr);
		}
	}
}

//--------------------------------------------------------------------------
Color getRottenColor(Color _color)
{
	return { (u8)(_color.r * 3 / 5), (u8)(128 + _color.g / 2), (u8)(_color.b * 3 / 5), _color.a };
}

//--------------------------------------------------------------------------
void Player::update(const float _dt)
{
	Entity::update(_dt);

	const auto & playerInfo = PlayerTypeInfo::get(m_playerType);
	const float rottenTimeLimit = (PlayerTypeInfo::Flags::StrongStomach & playerInfo.flags) ? 60.0f : 15.0f;

	if (m_isRotten && m_rottenTimer.getElapsedTime().asSeconds() > rottenTimeLimit)
	{
		if (!m_isAttacked)
		{
			m_isAttacked = true;
			m_damageTaken = false;
			m_damageType = PlayerDamageType::Rotten;
			m_timeSinceLastHit.restart();
			m_damagePending = -1;
			m_rottenTimer.restart();
		}
	}

	SuperVimontBros & game = SuperVimontBros::get();
	const auto & pad = getController();

	auto jDir = pad.getXYAxis();
	float jX = jDir.x;
	float jY = jDir.y;

	if (game.m_gameState != GameState::Play)
	{
		playAnimation(Animation::Idle);
		return;
	}

	const float timeSinceLastHit = m_timeSinceLastHit.getElapsedTime().asMilliseconds();

	bool takeDamage = false;
	if (m_isAttacked && timeSinceLastHit <= timeBeforeDamage + damageDuration)
	{
		if (timeSinceLastHit > timeBeforeDamage)
		{
			takeDamage = true;

			if (!m_damageTaken)
			{
				AddRemoveHP(m_damagePending);
				m_damageTaken = true;

				if (m_damageType == PlayerDamageType::Rotten)
				{
					playSound(SoundFX::Beurk);

					Splash * splash = new Splash(SplashType::PlayerThrowUp, 0);
					splash->init();
					splash->m_isColliderForOtherActors = false;
					splash->m_faceLeft = m_faceLeft;
					splash->m_animLeft = m_animLeft;
					splash->m_lutIndex = m_lutIndex;
					splash->m_shaderID = m_shaderID;

					int offset = 0;
					if (m_faceLeft)
						offset = -8;
					else
						offset = +10;

					splash->setPosition(getPosition().x + offset, getPosition().y, 0);
					splash->setColor(getRottenColor(splash->m_sprite.getColor()));

					//sf::Vector2f move = { 0.0f, 0.0f };
					//auto result = SuperVimontBros::get().tryMove(splash, move);
					//if (result.canMoveX && result.canMoveY)
						Game::get().registerVisual(splash);
					//else
					//	SAFE_RELEASE(splash);
				}
				else
				{
					playSound(SoundFX::Damage);
				}
			}
		}
	}
	else
	{
		m_isAttacked = false;
	}

	if (takeDamage && (m_damageType == PlayerDamageType::Zombie || m_damageType == PlayerDamageType::Bullet))
	{
		float alpha = 127 + (uint)(16 * 128 * timeSinceLastHit / (float)(damageDuration - timeBeforeDamage)) % 128;
		setColor({ 255, (u8)alpha, (u8)alpha, 255 });
	}
	else
	{
		setColor({ 255,255,255, isActive() ? (u8)255 : (u8)128 });
	}

	if (m_isRotten)
	{
		const auto & color = m_sprite.getColor();
		setColor(getRottenColor(color));
	}

	if (!canTakeDamage() && m_hasEverTakenDamage)
	{
		float alpha = 127 + (uint)(16 * 128 * timeSinceLastHit / (float)(damageDuration - timeBeforeDamage)) % 128;
		const auto color = getColor();
		setColor({ color.r, color.g, color.b, (u8)alpha });
	}

	float horizontalSpeed = 0.06f * _dt;
	float verticalSpeed = 0.035f * _dt;

	bool moving = false;
	bool punching = false;
	bool shiting = false;
	bool running = false;
	bool kicking = false;

	if (m_celebrate)
	{
		const uint celebrationTime = m_animTimer.getElapsedTime().asMilliseconds();
		if (celebrationTime >= getCurrentAnimationSequence().m_totalDuration * 10)
		{
			m_celebrate = false;
		}
	}

	if (pad.isButtonPressed((uint)Button::Run) && !isCarrying())
	{
		horizontalSpeed *= 1.75f;
		verticalSpeed *= 1.75f;
		running = true;
		m_celebrate = false;
	}

	if (game.m_debugDisplay)
	{
		// debug speed
		if (pad.isButtonPressed(7))
		{
			horizontalSpeed *= 10;
			verticalSpeed *= 10;
		}
	}

	const Vector2f previousPos = getPosition();

	if (isAnimationPlaying(Animation::Shit))
	{
		const uint shitTime = m_animTimer.getElapsedTime().asMilliseconds();

		addDebugText("Fart : %u\n", shitTime);

		if (pad.isButtonJustReleased((uint)Button::Shit) || shitTime > m_shitWarmupTime * 2)
		{
			if (shitTime > 250)
			{
				if (!m_shitDone)
				{
					bool bigOne = false;
					if (shitTime > m_shitWarmupTime)
					{
						bigOne = true;
					}

					if (bigOne && (playerInfo.flags & PlayerTypeInfo::Flags::AtomicFart))
					{
						game.addRadZone({ getPosition(), 96, this });
						playSound(SoundFX::Prout, 100);
					}
					else
					{
						ShitType shitType = ShitType::Default;

						if (bigOne && (playerInfo.flags & PlayerTypeInfo::Flags::ShitFootBall))
							shitType = ShitType::Ball;
						else if (bigOne && (playerInfo.flags & PlayerTypeInfo::Flags::ShitRugby))
							shitType = ShitType::Rugby;
						else if (bigOne && (playerInfo.flags & PlayerTypeInfo::Flags::ShitElectric))
							shitType = ShitType::Electric;

						Shit * shit = new Shit(shitType);

						int shitOffsetX = (m_faceLeft ? +3 : -1);

						if (playerInfo.flags & PlayerTypeInfo::Flags::IsADog)
						{
							shitOffsetX = (m_faceLeft ? +3 + 5 : -1 - 5);
						}

						shit->init();
						shit->setPosition(getPosition().x + shitOffsetX, getPosition().y, +4);
						shit->setParent(this);
						shit->setColor(PlayerTypeInfo::get(m_playerType).shitColor);

						Game::get().registerVisual(shit);
						shit->release();

						playSound(SoundFX::Prout);
					}

					m_shitDone = true;
				}
			}

			shiting = false;
		}
		else
		{
			shiting = true;
		}
	}
	else if (pad.isButtonPressed((uint)Button::Shit))
	{
		// start new shit
		if (pad.isButtonJustPressed((uint)Button::Shit))
		{
			shiting = true;
			m_shitDone = false;
			m_celebrate = false;
		}
	}

	if (m_animIndex == (AnimIndex)Animation::Punch)
	{
		if (!isCarrying())
		{
			if (m_animTimer.getElapsedTime().asMilliseconds() < (float)getCurrentAnimationSequence().m_totalDuration)
			{
				// still punching
				punching = true;
			}
		}
	}
	else if (pad.isButtonJustPressed((uint)Button::Punch))
	{
		punching = true;
		m_celebrate = false;

		// Enter car ? 
		if (!isInVehicle())
		{
			auto & cars = game.m_vehicles;
			Vehicle * closestCar = nullptr;
			float closetCarDist = 9999;
			for (uint i = 0; i < cars.size(); ++i)
			{
				Vehicle * car = cars[i];
				float dist = length(getPosition() - car->getPosition());
				if (dist < closetCarDist)
				{
					closestCar = car;
					closetCarDist = dist;
				}
			}

			if (closetCarDist < 32)
			{
				onEnterVehicle(closestCar);
			}
		}
		else
		{
			onExitVehicle();
		}
	}

	// find close actors
	auto list = findEntitiesInRadius(16);

	bool justPicked = false;

	if (!isInVehicle())
	{
		for (uint i = 0; i < list.size(); ++i)
		{
			auto * entity = list[i];

			Bonus * bonus = dynamic_cast<Bonus*>(entity);
			if (nullptr != bonus)
			{
				onBonusHit(bonus);
			}

			if (entity->isPickable() && (playerInfo.flags & PlayerTypeInfo::Flags::CanCarry) && !isCarrying() && !entity->isPicked())
			{
				if (pad.isButtonJustPressed((uint)Button::Punch))
				{
					pickEntity(entity);

					justPicked = true;
					punching = false;
					running = false;
				}
			}

			ElectricBox * box = dynamic_cast<ElectricBox*>(entity);
			if (box && pad.isButtonJustPressed((uint)Button::Punch))
			{
				if (!box->isOpen())
					box->open(this);
				else
					box->close();
			}
		}
	}

	bool drop = false;
	if (!justPicked && isCarrying())
	{
		if (pad.isButtonJustPressed((uint)Button::Punch))
		{
			drop = true;
		}
	}

	// Shoot
	if (m_animIndex == (AnimIndex)Animation::Kick && m_animTimer.getElapsedTime().asMilliseconds() < (float)getAnimationSequence(Animation::Kick).m_totalDuration)
	{
		kicking = true;
	}
	else if (pad.isButtonPressed((uint)Button::Kick))
	{
		const bool anyActorToKick = !isInVehicle();

		if (anyActorToKick || m_animIndex == (AnimIndex)Animation::PrepareKick)
		{
			if (pad.isButtonJustPressed((uint)Button::Kick))
			{
				debugPrint("Start Kicking\n");
				if (isCarrying())
				{
					m_entityToDropKick = getCurrentlyPickedEntity();
				}
			}

			kicking = true;
			m_kickDone = false;

			if (m_animIndex == (AnimIndex)Animation::PrepareKick)
			{
				float shootPower = min(1.0f, m_animTimer.getElapsedTime().asMilliseconds() / 1000.0f);

				addDebugText("%.0f%%\n", shootPower * 100.0f);
			}
		}

		m_celebrate = false;
	}
	else 
	{
		if (m_animIndex == (AnimIndex)Animation::PrepareKick)
		{
			kicking = true;
			m_kickDone = true;

			float shootPower = min(1.0f, m_animTimer.getElapsedTime().asMilliseconds() / 1000.0f);

			bool canDropKick = false;
			for (uint g = 0; g < game.m_goals.size(); ++g)
			{
				auto * goal = game.m_goals[g];

				switch (goal->getGoalType())
				{
					case GoalType::Football:
						canDropKick |= m_collisionAABB.intersects(goal->getGoalSurface());
						break;

					case GoalType::Rugby:
						canDropKick |= m_collisionAABB.intersects(goal->getHalfField());
						break;
				}
			}

			for (uint b = 0; b < list.size(); ++b)
			{
				Actor * actor = list[b];

				if (actor->m_kickSpeed > 0.0f)
				{
					Vector2f delta = actor->getPosition() - getPosition();
					float dist = length(delta);

					const float normJoyDir = length(jDir);
					if (normJoyDir > joyDeadZone)
					{
						delta = 1.0f * delta + 9.0f * jDir;
					}

					delta /= length(delta);

					const Ball * ball = dynamic_cast<Ball*>(actor);
					const Shit * shit = dynamic_cast<Shit*>(actor);

					float shotHeight = 0.1f;
					float shotNerf = 1.0f;

					if (actor == m_entityToDropKick && ball)
					{
						if (1)//canDropKick)
						{
							if (ball->isRugbyBall())
							{
								shotHeight = 0.35f;
								shotNerf = 0.35f;
							}
							else if (ball->isFootBall())
							{
								shotHeight = 0.20f;
								shotNerf = 0.40f;
							}
							else if (ball->isTennisBall())
							{
								shotHeight = 0.215f;
								shotNerf = 0.85f;
							}
							else
							{
								shotHeight = 0.15f;
								shotNerf = 0.5f;
							}
						}
						
						dropPickedEntity();
						m_entityToDropKick = nullptr;
					}

					actor->m_velocity = actor->m_velocity * 0.5f + delta * shootPower * actor->m_kickSpeed * shotNerf;
					actor->m_jump = clamp(shootPower - 0.25f, 0.0f, 0.65f) * shotHeight;
					actor->setParent(this);
					actor->playSound((SoundIndex)SoundFX::Hit, 25 + shootPower * 75);
				}
			}
		}
	}

	Vector2f moveVec = {0,0};

	if (!punching && !shiting && !kicking && !m_vehicle)
	{
		if (jX > joyDeadZone)
		{
			moveVec += (Vector2f(1.0f, 0.0f) * horizontalSpeed);
			moving = true;
			m_faceLeft = false;
		}
		else if (jX < -joyDeadZone)
		{
			moveVec += (-Vector2f(1.0f, 0.0f) * horizontalSpeed);
			moving = true;
			m_faceLeft = true;
		}

		if (jY > joyDeadZone)
		{
			moveVec += (Vector2f(0.0f, 1.0f) * verticalSpeed);
			moving = true;
		}
		else if (jY < -joyDeadZone)
		{
			moveVec += (-Vector2f(0.0f, 1.0f) * verticalSpeed);
			moving = true;
		}

		// Only if fullscreen viewport ! TODO: viewport
		if (game.m_editorMode == EditorMode::None)
		{
			clampPositionToLevel(game.m_level);
			clampPositionToViewport(*m_viewport);
		}		
	}

	if (!m_isAttacked || (m_damageType != PlayerDamageType::Electricity && m_damageType != PlayerDamageType::Rotten))
	{
		if (moving)
		{
			// Try to move player
			if (game.tryMove(this, moveVec))
			{
				move(moveVec);
			}
		}
	}

	if (abs(previousPos.x - getPosition().x) < 0.01f && abs(previousPos.y - getPosition().y) < 0.01f)
	{
		moving = false;
	}

	if (moving || kicking || punching || shiting)
	{
		m_celebrate = false;
	}

	if (moving)
	{		
		if (isCarrying())
		{
			playAnimation(Animation::Carry, m_faceLeft);
		}
		else
		{
			playAnimation(running ? Animation::Run : Animation::Walk, m_faceLeft);
		}
	}
	else if (m_isAttacked)
	{
		switch (m_damageType)
		{
			case PlayerDamageType::Rotten:
				playAnimation(Animation::ThrowUp, m_animLeft);
				break;

			case PlayerDamageType::Zombie:
				playAnimation(Animation::Die, m_animLeft);
				break;

			case PlayerDamageType::Electricity:
				playAnimation(Animation::Electricity, m_animLeft);
				break;
		}
		
		m_hasEverTakenDamage = true;
		drop = true;
	}
	else if (kicking)
	{
		if (m_kickDone)
		{
			playAnimation(Animation::Kick, m_faceLeft);
		}
		else
		{
			playAnimation(Animation::PrepareKick, m_faceLeft);
		}

		//drop = true;
	}
	else if (punching)
	{
		playAnimation(Animation::Punch, m_faceLeft);

		drop = true;
	}
	else if (shiting)
	{
		playAnimation(Animation::Shit, m_faceLeft);

		drop = true;
	}
	else if (m_celebrate)
	{
		playAnimation(Animation::CelebrateGoal, m_faceLeft);

		drop = true;
	}
	else
	{
		if (isCarrying())
		{
			playAnimation(Animation::IdleCarry, m_faceLeft);
		}
		else
		{
			playAnimation(Animation::Idle, m_faceLeft);
		}
	}	

	if (drop)
	{
		m_entityToDropKick = nullptr;
		dropPickedEntity();
	}

	if (isCarrying())
	{
		updatePickedEntityPos();		
	}

	if (m_hp != m_lastHPFade)
	{
		if (m_hp == 3 && m_lastHPFade != 3)
		{
			m_viewport->getPostProcess()->m_saturation.set(0.75f, 0.25f);
			m_lastHPFade = 3;
		}
		else if (m_hp == 2 && m_lastHPFade != 2)
		{
			m_viewport->getPostProcess()->m_saturation.set(0.5f, 0.25f);
			m_lastHPFade = 2;
		}
		else if (m_hp == 1 && m_lastHPFade != 1)
		{
			m_viewport->getPostProcess()->m_saturation.set(0.25f, 0.25f);
			m_lastHPFade = 1;
		}
		else if (m_hp == 0 && m_lastHPFade != 0)
		{
			m_viewport->getPostProcess()->m_saturation.set(0.0f, 0.25f);
			m_lastHPFade = 0;
		}
		else
		{
			m_viewport->getPostProcess()->m_saturation.set(1.0f, 0.25f);
			m_lastHPFade = m_hp;
		}
	}
}

//--------------------------------------------------------------------------
void Player::drawShadow(sf::RenderTexture & _surface)
{
	Entity::drawShadow(_surface);
}

//--------------------------------------------------------------------------
void Player::draw(RenderTexture & _surface)
{
	SuperVimontBros & game = SuperVimontBros::get();

	if (m_animIndex == (AnimIndex)Animation::PrepareKick)
	{
		Vector2f joyDir = getController().getXYAxis();; // Vector2f(Joystick::getAxisPosition(m_playerIndex, Joystick::Axis::X), Joystick::getAxisPosition(m_playerIndex, Joystick::Axis::Y));

		float normJoyDir = length(joyDir);

		if (normJoyDir > joyDeadZone)
		{
			joyDir /= normJoyDir;

			float angle = 0;

			if (joyDir.y > 0)
			{
				angle = acos(joyDir.x);
				angle *= 180.0f / 3.1417f;
				angle += 90.0f;
			}
			else 
			{
				angle = acos(joyDir.x);
				angle *= 180.0f / 3.14f;
				angle = 180.0f - (angle+90.0f);
			}

			// TODO: Fix Animation being reset by animation change
			if (angle >= 0 && angle <= 180)
			{
				m_faceLeft = false;
			}
			else
			{
				m_faceLeft = true;
			}

			const Vector2u arrowCoords = { 4,15 };

			TileInfo & info = game.m_spritesTile.m_tileInfos.get(arrowCoords);

			Sprite arrow;
			arrow.setTexture(game.m_spritesTile.m_texture);
			arrow.setTextureRect(game.m_spritesTile.getTileRect(arrowCoords));
			arrow.setPosition(getPosition());
			
			arrow.setOrigin((Vector2f)info.m_origin - Vector2f(info.m_box.left + 15, info.m_box.top + 15));

			float shootPower = min(1.0f, m_animTimer.getElapsedTime().asMilliseconds() / 1000.0f);

			arrow.setScale(0.75f + 0.25f * shootPower, 0.5f + shootPower * 1.0f);

			arrow.setColor({ 255, (u8)(255 - shootPower * 128), (u8)(255 - shootPower * 255),(u8)(64 + shootPower * 64) });

			arrow.setRotation(angle);

			_surface.draw(arrow);
		}		
	}

	Entity::draw(_surface);
}