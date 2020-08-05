#include "Precomp.h"
#include "Enemy.h"
#include "Entity/Shit/Shit.h"
#include "Entity/Ball/Ball.h"
#include "Entity/Shit/Shit.h"
#include "Entity/Goal/Goal.h"
#include "Entity/Bullet/Bullet.h"
#include "Entity/Player/Player.h"
#include "Entity/Splash/Splash.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Sound/SoundBank.h"
#include "Viewport/Viewport.h"
#include "Tiles/SpriteModel.h"

using namespace sf;

sf::Sound * Enemy::s_lastGrrSound = nullptr;

static const float start_fire_dist = 256;
static const float start_fire_angle = 0.10f;
static const float stop_fire_dist = 320;
static const float stop_fire_angle = 0.15f;
static const float prepare_fire_delay = 700;

//--------------------------------------------------------------------------
const EnemyTypeInfo & EnemyTypeInfo::get(EnemyType _enemyType)
{
	static EnemyTypeInfo enemyTypeInfo[] =
	{
		EnemyTypeInfo("Zombie",		SPRITE_LINE(SpriteModel::Zombie),		ChasePlayers,							    0.021f, 500, 384, 16),
		EnemyTypeInfo("Footballer",	SPRITE_LINE(SpriteModel::Footballer),	ChasePlayers | AnimLeft | ChaseFootball,	0.023f, 200, 256, 32),
		EnemyTypeInfo("Footballer2",SPRITE_LINE(SpriteModel::Footballer2),	ChasePlayers |            ChaseFootball,	0.024f, 200, 256, 32),
		EnemyTypeInfo("Goalkeeper", SPRITE_LINE(SpriteModel::Goalkeeper),	GoalKeeper,									0.030f, 150, 512, 64),
		EnemyTypeInfo("Goalkeeper2",SPRITE_LINE(SpriteModel::Goalkeeper2),	GoalKeeper,									0.032f, 100, 512, 64),
		EnemyTypeInfo("Rugbyman",	SPRITE_LINE(SpriteModel::Rugbyman),		ChasePlayers | AnimLeft | ChaseRugbyBall,	0.018f, 300, 256, 32),
		EnemyTypeInfo("YellowVest",	SPRITE_LINE(SpriteModel::YellowVest),	ChasePlayers,							    0.015f, 700, 192, 24),
		EnemyTypeInfo("Plagiste",	SPRITE_LINE(SpriteModel::ZombiePlaya),  ChasePlayers,								0.025f, 256, 784, 20),
		EnemyTypeInfo("Cowboy",		SPRITE_LINE(SpriteModel::ZombieCowboy), ChasePlayers,								0.029f, 300, 800, 22),
		EnemyTypeInfo("Patrick",	SPRITE_LINE(SpriteModel::ZombiePatrick),ChasePlayers | CanFire,						0.020f, 200, 512, 32),
	};
	static_assert(COUNT_OF(enemyTypeInfo) == (uint)EnemyType::Count);

	assert((uint)_enemyType < (uint)EnemyType::Count);
	return enemyTypeInfo[(uint)_enemyType];
}

//--------------------------------------------------------------------------
Enemy::Enemy(EnemyType _enemyType) :
	Entity(EnemyTypeInfo::get(_enemyType).name, Game::get().m_spritesTile),
	m_enemyType(_enemyType)
{
	m_isColliderForOtherActors = true;

	m_hasAnimLeft = testFlags(EnemyTypeInfo::Flags::AnimLeft);

	SuperVimontBros::get().m_enemies.add(this);
}

//--------------------------------------------------------------------------
Enemy::~Enemy()
{
	SuperVimontBros::get().m_enemies.remove(this);
}

//--------------------------------------------------------------------------
bool Enemy::testFlags(EnemyTypeInfo::Flags _flags) const
{
	const auto & info = getInfo();
	return info.flags & _flags;
}

//--------------------------------------------------------------------------
void Enemy::init()
{
	Entity::init();

	m_side = Goal::getTerrainSide(this);

	const auto & info = EnemyTypeInfo::get(m_enemyType);

	const uint line = info.line;

	AnimationSequence & idle = getAnimationSequence(Animation::Idle);
						idle.addFrame(AnimFrame({0,line }));

	AnimationSequence & walk = getAnimationSequence(Animation::Walk);
						walk.addFrame(AnimFrame({ 1,line }, 400));
						walk.addFrame(AnimFrame({ 2,line }, 400));

	AnimationSequence & run = getAnimationSequence(Animation::Run);
						run.addFrame(AnimFrame({ 1,line }, 200));
						run.addFrame(AnimFrame({ 2,line }, 200));
						run.addFrame(AnimFrame({ 1,line }, 200));
						run.addFrame(AnimFrame({ 3,line }, 200));
						run.addFrame(AnimFrame({ 4,line }, 200));
						run.addFrame(AnimFrame({ 3,line }, 200));

	AnimationSequence & die = getAnimationSequence(Animation::Die);
						die.addFrame(AnimFrame({ 9,line }, 250));
						die.addFrame(AnimFrame({10,line }, 250));

	AnimationSequence & fire = getAnimationSequence(Animation::Fire);
						fire.addFrame(AnimFrame({ 22,line }, prepare_fire_delay));
						fire.addFrame(AnimFrame({ 23,line }, 100));

	AnimationSequence & electricity = getAnimationSequence(Animation::Electricity);
	for (uint i = 0; i < 5; ++i)
	{
		electricity.addFrame(AnimFrame({ 8,line }, 50));
		electricity.addFrame(AnimFrame({ 9,line }, 50));
	}

	AnimationSequence & punch = getAnimationSequence(Animation::Punch);
						punch.addFrame(AnimFrame({ 6,line }, 250));
						punch.addFrame(AnimFrame({ 7,line }, 250));
						punch.addFrame(AnimFrame({ 6,line }, 250));
						punch.addFrame(AnimFrame({ 7,line }, 250));
						punch.addFrame(AnimFrame({ 6,line }, 250));
						punch.addFrame(AnimFrame({ 7,line }, 250));
						punch.addFrame(AnimFrame({ 6,line }, 250));
						punch.addFrame(AnimFrame({ 7,line }, 250));

	AnimationSequence & idleCarry = getAnimationSequence(Animation::IdleCarry);
						idleCarry.addFrame(AnimFrame({ 12, line }, 200));

	AnimationSequence & carry = getAnimationSequence(Animation::Carry);
						carry.addFrame(AnimFrame({ 16, line }, 200));
						carry.addFrame(AnimFrame({ 17, line }, 200));
						carry.addFrame(AnimFrame({ 18, line }, 200));
						carry.addFrame(AnimFrame({ 19, line }, 200));

	AnimationSequence & prepareKick = getAnimationSequence(Animation::PrepareKick);
						prepareKick.addFrame(AnimFrame({ 6,line }, 250));

	AnimationSequence & kick = getAnimationSequence(Animation::Kick);
						kick.addFrame(AnimFrame({ 7,line }, 250));

	if (info.flags & EnemyTypeInfo::Flags::GoalKeeper)
		m_faceLeft = false;
	else
		m_faceLeft = true;

	m_animLeft = m_faceLeft;

	addCustomSoundFX(SoundFX::GrrrA, "SuperVimontBros/data/sound", "Grrr.wav");
	addCustomSoundFX(SoundFX::GrrrB, "SuperVimontBros/data/sound", "Grrr2.wav");
	addCustomSoundFX(SoundFX::Damage, "SuperVimontBros/data/sound", "aie.wav");
	
	if (testFlags(EnemyTypeInfo::Flags::CanFire))
		addCustomSoundFX(SoundFX::Fire, "SuperVimontBros/data/sound", "Fire.wav");
}

//--------------------------------------------------------------------------
bool Enemy::setAttacking()
{
	enterState(EnemyState::Attack);
	return true;
}

//--------------------------------------------------------------------------
bool Enemy::kill(Player * _byPlayer, KillCause _cause)
{
	if (EnemyState::Die != m_enemyState)
	{
		SplashType splashType;
		ShitType shitType;
		uint score;

		switch (_cause)
		{
		default:
			splashType = SplashType::Enemy;
			if (m_enemyType == EnemyType::ZombiePatrick)
			{
				shitType = ShitType::PatrickHead;
				score = 500;
			}
			else
			{
				shitType = ShitType::ZombieHead;
				score = 50;
			}
			break;

		case KillCause::Electricity:
			if (m_enemyType == EnemyType::ZombiePatrick)
				score = 1000;
			else
				score = 250;
			splashType = SplashType::Cinder;
			shitType = ShitType::ZombieHeadSkeleton;
			break;
		}

		m_killCause = _cause;
		playSound(SoundFX::Damage, 25);
		if (_byPlayer)
		{
			_byPlayer->addZombieKilled(1);
			_byPlayer->addScore(score);
			
		}
		enterState(EnemyState::Die);

		Splash * splash = new Splash(splashType, (uint)m_enemyType);
		splash->init();
		splash->m_isColliderForOtherActors = false;
		splash->m_faceLeft = m_faceLeft;
		splash->m_animLeft = m_animLeft;
		splash->m_lutIndex = m_lutIndex;
		splash->m_shaderID = m_shaderID;
		splash->setPosition(getPosition().x, getPosition().y, 0);
		Game::get().registerVisual(splash);

		Shit * head = new Shit(shitType);
		head->init();
		head->m_isColliderForOtherActors = false;
		head->m_faceLeft = m_faceLeft;
		head->m_animLeft = m_animLeft;
		head->m_lutIndex = m_lutIndex;
		head->m_shaderID = m_shaderID;
		head->setPosition(getPosition().x, getPosition().y, 8);
		Game::get().registerVisual(head);
		head->release();

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
void Enemy::onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	Player * player = dynamic_cast<Player*>(_other);
	if (player)
	{
		player->onEnemyHit(this);
	}

	Shit * shit = dynamic_cast<Shit*>(_other);
	if (shit)
	{
		onShitHit(shit);
		return;
	}

	Super::onActorCollision(_other, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
bool Enemy::onShitHit(Shit * _shit)
{
	KillCause cause = KillCause::Default;

	if (ShitType::Electric == _shit->getShitType())
		cause = KillCause::Electricity;

	kill(dynamic_cast<Player*>(_shit->getParent()), cause);
	return true;
}

//--------------------------------------------------------------------------
bool Enemy::onBulletHit(Bullet * _bullet)
{
	kill(dynamic_cast<Player*>(_bullet->getParent()), KillCause::Bullet);
	return true;
}

//--------------------------------------------------------------------------
bool Enemy::onHitBall(Ball * _ball)
{
	// Balls are killing enemies only if moving
	if (_ball->isMoving() && _ball->getParent() != this && !asBool(EnemyTypeInfo::get(m_enemyType).flags & EnemyTypeInfo::Flags::GoalKeeper))
	{
		kill(dynamic_cast<Player*>(_ball->getParent()));
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
const char * getEnemyStateName(EnemyState _enemyState)
{
	const char * enemyStateNames[] =
	{
		"IDLE",
		"WAIT",
		"PLAYER",
		"BALL",
		"ATTACK",
		"DIE",
		"PREPARE",
		"KICK",
		"STRAFE",
		"FIRE",
	};
	static_assert(COUNT_OF(enemyStateNames) == (uint)EnemyState::Count);
	return enemyStateNames[(uint)_enemyState];
}

//--------------------------------------------------------------------------
template <class T> bool sortByDistance(const FindResult<T> & _A, const FindResult<T> & _B)
{
	return _A.dist < _B.dist;
}

//--------------------------------------------------------------------------
// Return sorted list of entities in radius
//--------------------------------------------------------------------------
template <class T> std::vector<T*> Enemy::findEntitiesInRadius(const std::vector<T*> & _from, float _radius)
{
	const auto & pos = getPosition();
	std::vector<T*> closeEntities;
	for (auto * entity : _from)
	{
		Vector2f delta = entity->getPosition() - pos;
		const float dist = length(delta);

		if (dist < _radius)
		{
			closeEntities.push_back(entity);
		}
	}

	std::sort(closeEntities.begin(), closeEntities.end(), [&pos](T * _a, T * _b) { return length(_a->getPosition() - pos) > length(_b->getPosition() - pos); });
	return closeEntities;
}

//--------------------------------------------------------------------------
// Return 'true' if _entity is the closer to _target than any _others in _radius (optional)
//--------------------------------------------------------------------------
template <class T> bool Enemy::isCloserThan(const Entity * _entity, const Entity * _target, const std::vector<T*> & _others, float _radius)
{
	const auto & ePos = _entity->getPosition();
	const float d = length(ePos - _target->getPosition());
	const auto & tPos = _target->getPosition();

	for (const auto * other : _others)
	{
		const auto & oPos = other->getPosition();
		const float eDist = length(oPos - ePos);
		if (eDist < _radius)
		{
			const float tDist = length(oPos - tPos);
			if (tDist < d)
			{
				return false;
			}
		}
	}
	return true;
}

//--------------------------------------------------------------------------
template <class T> FindResult<T> Enemy::findClosest(const Entity * _entity, const std::vector<T*> & _from)
{
	FindResult<T> result;

	const auto pos = _entity->getPosition();

	for (const T * entity : _from)
	{
		const Vector2f delta = entity->getPosition() - pos;
		float dist = length(delta);

		if ((Enemy*)entity == _entity)
			continue;

		if (dist < result.dist)
		{
			result.entity = entity;
			result.dist = dist;
			result.dir = (dist != 0.0f) ? delta / dist : Vector2f(0.0f, 0.0f);
		}
	}

	return result;
}

//--------------------------------------------------------------------------
template <class T> FindResult<T> Enemy::findClosest_if(const Entity * _entity, const std::vector<T*> & _from, std::function<bool(const T*)> _condition)
{
	FindResult<T> result;

	const auto pos = _entity->getPosition();

	for (const T * entity : _from)
	{
		const Vector2f delta = entity->getPosition() - pos;
		const float dist = length(delta);

		if ((Enemy*)entity == _entity)
			continue;

		if (dist < result.dist)
		{
			if (_condition(entity))
			{
				result.entity = entity;
				result.dist = dist;
				result.dir = (dist != 0.0f) ? delta / dist : Vector2f(0.0f, 0.0f);
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------
bool Enemy::isVisibleInAnyViewport(const Entity * _entity)
{
	SuperVimontBros & game = SuperVimontBros::get();
	for (auto * viewport : game.m_viewports)
	{
		if (viewport->isEnabled())
		{
			if (viewport->isVisible(_entity))
				return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
bool Enemy::updateLookAt(const sf::Vector2f & _moveDir, float _dt)
{
	if (getInfo().flags & EnemyTypeInfo::Flags::GoalKeeper)
		return false;

	if (m_faceLeft && _moveDir.x > 0.01f * _dt)
	{
		m_faceLeft = false;
		return true;
	}
	else if (!m_faceLeft && _moveDir.x < -0.01f * _dt)
	{
		m_faceLeft = true;
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
void Enemy::enterState(EnemyState _newState)
{
	const auto previousState = m_enemyState;
	if (_newState != previousState)
	{
		if (m_killCause != (KillCause)0 && _newState != EnemyState::Die)
		{
			return; // antibug
		}

		m_enemyState = _newState;

		switch (m_enemyState)
		{
			case EnemyState::Player:
				if ((!s_lastGrrSound || s_lastGrrSound->getStatus() == SoundSource::Stopped) && isVisibleInAnyViewport(this))
				{
					playSound(SoundFX::GrrrA);
					s_lastGrrSound = getSoundInstance();
				}
				break;

			case EnemyState::Ball:
				if ((!s_lastGrrSound || s_lastGrrSound->getStatus() == SoundSource::Stopped) && isVisibleInAnyViewport(this))
				{
					playSound(SoundFX::GrrrB);
					s_lastGrrSound = getSoundInstance();
				}
				break;

			case EnemyState::PrepareShoot:
				m_waitTimer.restart();
				break;

			case EnemyState::Kick:
				m_waitTimer.restart();
				break;

			case EnemyState::Wait:
				m_waitTimer.restart();
				break;

			case EnemyState::Die:
				fadeOutAndKill(500);
				m_isColliderForOtherActors = false;
				if (isCarrying())
				{
					dropPickedEntity();
				}
				break;

			case EnemyState::Fire:
				m_waitTimer.restart();
				break;
		}
	}
}

//--------------------------------------------------------------------------
bool Enemy::findGoalDir(sf::Vector2f & _dir, Entity * _entity, GoalType _goalType, GoalSide _goalSide)
{
	SuperVimontBros & game = SuperVimontBros::get();
	for (auto * goal : game.m_goals.get())
	{
		if (goal->getGoalType() == _goalType && goal->getGoalSide() == _goalSide)
		{
			_dir = normalize( goal->getPosition() - _entity->getPosition());

			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
template <class T> FindResult<T> Enemy::findClosest_alive(const T * _entity, const std::vector<T*> & _from)
{
	FindResult<T> result;

	const auto pos = _entity->getPosition();

	for (const T * entity : _from)
	{
		if (entity && _entity->getEnemyState() != EnemyState::Die)
		{
			const Vector2f delta = entity->getPosition() - pos;
			const float dist = length(delta);

			if (dist < result.dist)
			{
				result.entity = entity;
				result.dist = dist;
				result.dir = (dist != 0.0f) ? delta / dist : Vector2f(0.0f, 0.0f);
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------
void Enemy::update(const float _dt)
{
	SuperVimontBros & game = SuperVimontBros::get();
	const auto & info = getInfo();

	Entity::update(_dt);

	const float kickDist = 16;

	std::function<bool(const Player*)> isPlayerAlive = [](const Player * _player) { return _player->m_hp > 0; };
	std::function<bool(const Enemy*)> isEnemyAlive = [](const Enemy * _enemy) { return _enemy->getEnemyState() != EnemyState::Die; };

	const FindResult<Player> closestPlayer = findClosest_if(this, game.m_players, isPlayerAlive);
	const FindResult<Enemy> closestEnemy = findClosest_alive(this, game.m_enemies.get());

	FindResult<Ball> closestBall;

	const bool footballer = info.flags & EnemyTypeInfo::Flags::ChaseFootball;
	const bool rugbyman = info.flags & EnemyTypeInfo::Flags::ChaseRugbyBall;
	const bool goalKeeper = info.flags & EnemyTypeInfo::Flags::GoalKeeper;

	if (footballer || goalKeeper)
		closestBall = findClosest(this, game.m_footballs);
	else if (rugbyman)
		closestBall = findClosest(this, game.m_rugbyBalls);

	Vector2f moveDir = { 0.0f, 0.0f };
	bool moving = false;

	const auto id = getGUID();
	
	if ( (m_side == GoalSide::Home || m_side == GoalSide::Away) && asBool(info.flags & (EnemyTypeInfo::Flags::ChaseFootball | EnemyTypeInfo::Flags::ChaseRugbyBall) ))
	{
		addDebugText("#%u (%s)\n", id, m_side == GoalSide::Home ? "Home" : "Away");
	}
	else
	{
		addDebugText("#%u\n", id);
	}

	if (!goalKeeper)
	{
		for (auto & zone : game.m_radZones)
		{
			auto dist = length(getPosition() - zone.position);
			if (dist < zone.radius)
			{
				kill(zone.owner);
			}
		}
	}

	if (isVisibleInAnyViewport(this))
	{
		switch (m_enemyState)
		{
		case EnemyState::Idle:
		{
			if ( (footballer||rugbyman) && closestBall.found() && closestBall.dist < info.m_walkMaxDist && !isCarrying() && !closestBall.entity->isPicked() && isCloserThan(this, closestBall.entity, game.m_enemies.get()))
			{
				if (closestBall.dist < kickDist && footballer)
					enterState(EnemyState::PrepareShoot);
				else
					enterState(EnemyState::Ball);
			}
			else if (closestPlayer.found() && closestPlayer.dist < info.m_walkMaxDist && (info.flags & EnemyTypeInfo::Flags::ChasePlayers))
			{
				enterState(EnemyState::Player);
			}
			else if (info.flags & EnemyTypeInfo::Flags::GoalKeeper)
			{
				if (closestBall.dist < kickDist && asBool(info.flags & (EnemyTypeInfo::Flags::ChaseFootball| EnemyTypeInfo::Flags::GoalKeeper)) && closestBall.entity->isFootBall())
					enterState(EnemyState::PrepareShoot);
				else if (closestBall.found() && abs(closestBall.entity->getPosition().x - getPosition().x) > 8)
				{
					Goal * myGoal = nullptr;
					for (auto * goal : game.m_goals.get())
					{
						if (goal->getGoalType() == GoalType::Football && goal->getGoalSide() == m_side)
						{
							myGoal = goal;
							break;
						}
					}

					const auto x = closestBall.entity->getPosition().x;					

					if (myGoal)
					{
						const TileInfo & tile = myGoal->getTileInfo();
						const Vector2f & goalPos = myGoal->getPosition();
						const float xMin = goalPos.x + myGoal->m_offsetX - (float)tile.m_box.width / 2.0f;
						const float xMax = goalPos.x + myGoal->m_offsetX + (float)tile.m_box.width / 2.0f;

						if (x > xMin && x < xMax)
							enterState(EnemyState::Strafe);
					}
				}
			}
		}
		break;

		case EnemyState::Strafe:
		{
			if (closestBall.found() && closestBall.dist < info.m_walkMaxDist)
			{
				moveDir = _dt * closestBall.dir * info.m_speed;
				moveDir.y = 0;

				if (game.tryMove(this, moveDir))
				{
					if (length(moveDir) > 0.001f * _dt)
					{
						move(moveDir);
						moving = true;
					}
				}

				if (closestBall.dist < kickDist && asBool(info.flags & (EnemyTypeInfo::Flags::ChaseFootball | EnemyTypeInfo::Flags::GoalKeeper)) && closestBall.entity->isFootBall())
					enterState(EnemyState::PrepareShoot);
				else if (!moving)
					enterState(EnemyState::Wait);
			}
			else
			{
				enterState(EnemyState::Wait);
			}
		}
		break;

		case EnemyState::Wait:
		{
			const uint time = m_waitTimer.getElapsedTime().asMilliseconds();
			const bool otherEnemyClose = false; // closestEnemy.dist < info.m_distanceToOtherEnemies; TODO: closest shouldn't block
			if (time >= info.m_waitBeforeWalk && !otherEnemyClose)
			{
				enterState(EnemyState::Idle);
			}
			else
			{
				addDebugText("%.0u\n", time);
			}
		}
		break;

		case EnemyState::Attack:
		{
			const uint time = m_animTimer.getElapsedTime().asMilliseconds();
			const uint duration = getAnimationSequence(Animation::Punch).m_totalDuration;
			if (time >= duration)
			{
				enterState(EnemyState::Wait);
			}
		}
		break;

		case EnemyState::PrepareShoot:
			if (m_waitTimer.getElapsedTime().asMilliseconds() > 500)
				enterState(EnemyState::Kick);
			break;

		case EnemyState::Kick:
			if (m_waitTimer.getElapsedTime().asMilliseconds() > 250)
			{
				if (closestBall.dist < kickDist)
				{
					Ball * ball = (Ball*)closestBall.entity;

					float shootPower, shotHeight;

					bool goalDirFound = false;
					Vector2f goalDir;
					if (goalKeeper)
					{
						shootPower = 0.35f + frand() * 1.1f;
						shotHeight = 0.15f + frand() * 0.15f;
					}
					else
					{
						shootPower = 0.25f + frand() * 1.0f;
						shotHeight = 0.05f + frand() * 0.15f;

						if (m_side == GoalSide::Away /*&& asBool(terrainTest & TerrainTestFlags::IsInFootballDownHalf)*/)
							goalDirFound = findGoalDir(goalDir, this, GoalType::Football, GoalSide::Home);
						else if (m_side == GoalSide::Home /*&& asBool(terrainTest & TerrainTestFlags::IsInFootballUpHalf)*/)
							goalDirFound = findGoalDir(goalDir, this, GoalType::Football, GoalSide::Away);
					}

					Vector2f shootDir = closestBall.dir;

					if (goalDirFound)
					{
						shootDir = lerp(shootDir, goalDir, 0.5f + frand() * 0.5f);
					}

					ball->m_velocity = ball->m_velocity * 0.5f + shootDir * shootPower * ball->m_kickSpeed;
					ball->m_jump = clamp(shootPower - 0.25f, 0.0f, 0.65f) * shotHeight;
					ball->setParent(this);
					ball->playSound(SoundFX::Hit, (int)(25 + shootPower * 75));
				}

				enterState(EnemyState::Wait);
			}
			break;

		case EnemyState::Ball:
		{
			if (closestBall.found())
			{
				moveDir = _dt * closestBall.dir * info.m_speed;
				if (game.tryMove(this, moveDir))
				{
					if (length(moveDir) > 0.01f * _dt)
					{
						move(moveDir);
						moving = true;
					}
				}
				if (!moving)
				{
					enterState(EnemyState::Wait);
				}

				Ball * ball = (Ball*)closestBall.entity;
				if (closestBall.dist < 16 && ball->isPickable() && !ball->isPicked() && !isCarrying() && length(ball->m_velocity) <= 0.001f)
				{
					if (ball->isRugbyBall() && (info.flags & EnemyTypeInfo::Flags::ChaseRugbyBall))
					{
						pickEntity(ball);
						enterState(EnemyState::Wait);
					}
				}
				else if (ball->isPicked())
				{
					enterState(EnemyState::Wait);
				}
			}
			else
			{
				enterState(EnemyState::Wait);
			}
		}
		break;

		case EnemyState::Die:
			break;

		case EnemyState::Fire:
		{
			if (m_waitTimer.getElapsedTime().asMilliseconds() > prepare_fire_delay)
			{
				playSound(SoundFX::Fire);

				Bullet * bullet = new Bullet(BulletType::Default);
				bullet->init();
				bullet->setParent(this);
				bullet->m_isColliderForOtherActors = true;
				bullet->m_faceLeft = m_faceLeft;
				bullet->m_animLeft = m_animLeft;
				bullet->m_lutIndex = m_lutIndex;
				bullet->m_shaderID = m_shaderID;
				bullet->setPosition(getPosition().x, getPosition().y, 0);

				const float bullet_speed = 0.4f;

				const float random = ((float(rand() & 255) / 255.0f) - 0.5f) * bullet_speed * 0.1f;
				float y = random;
				if (closestPlayer.found())
					y = 0.5f * random + 0.5f * closestPlayer.dir.y;

				bullet->m_velocity = { m_faceLeft ? -bullet_speed : bullet_speed, y };
				Game::get().registerVisual(bullet);
				bullet->release();

				m_waitTimer.restart();
			}
			else if (closestPlayer.dist > stop_fire_dist || abs(closestPlayer.dir.y) > stop_fire_angle)
			{
				enterState(EnemyState::Wait);
			}
		}
		break;

		case EnemyState::Player:
		{
			bool chase = closestPlayer.found() && closestPlayer.dist < info.m_walkMaxDist;

			if (chase)
			{
				if (closestEnemy.dist < info.m_distanceToOtherEnemies && !isCloserThan(this, closestPlayer.entity, game.m_enemies.get(), info.m_distanceToOtherEnemies))
					chase = false;

				if (info.flags & (EnemyTypeInfo::Flags::ChaseFootball | EnemyTypeInfo::Flags::ChaseRugbyBall))
				{
					const auto flags = Goal::isInsideField(closestPlayer.entity);

					if (info.flags & EnemyTypeInfo::Flags::ChaseFootball && !asBool(flags & TerrainTestFlags::IsInFootballField))
						chase = false;

					if (info.flags & EnemyTypeInfo::Flags::ChaseRugbyBall && !asBool(flags & TerrainTestFlags::IsInRugbyField))
						chase = false;
				}
			}

			if (testFlags(EnemyTypeInfo::Flags::CanFire))
			{
				if (closestPlayer.dist < start_fire_dist && abs(closestPlayer.dir.y) < start_fire_angle)
				{
					enterState(EnemyState::Fire);
					break;
				}
			}

			// Chase the closest player
			if (chase)
			{
				moveDir = _dt * closestPlayer.dir * info.m_speed;
				if (game.tryMove(this, moveDir))
				{
					if (length(moveDir) > 0.01f * _dt)
					{
						move(moveDir);
						moving = true;
					}
				}

				// If blocked then wait
				if (!moving && m_enemyState != EnemyState::Attack)
				{
					enterState(EnemyState::Wait);
				}
			}
			else
			{
				// If no player found or if player is too far then wait then wait
				enterState(EnemyState::Wait);
			}
		}
		break;
		}
	}
	addDebugText("%s\n", getEnemyStateName(m_enemyState));

	// Face left/right
	if (moving)
	{
		if (updateLookAt(moveDir, _dt))
			enterState(EnemyState::Wait);
	}

	switch (m_enemyState)
	{
		case EnemyState::Strafe:
			playAnimation(Animation::Run, m_faceLeft);
			break;

		case EnemyState::Fire:
			playAnimation(Animation::Fire, m_faceLeft);
			break;

		case EnemyState::Wait:
		case EnemyState::Idle:
			if (isCarrying())
				playAnimation(Animation::IdleCarry, m_faceLeft);
			else
				playAnimation(Animation::Idle, m_faceLeft);
			break;

		case EnemyState::Player:
		case EnemyState::Ball:
			if (moving)
			{
				if (isCarrying())
					playAnimation(Animation::Carry, m_faceLeft);
				else
					playAnimation(Animation::Walk, m_faceLeft);
			}
			else
			{
				if (isCarrying())
					playAnimation(Animation::IdleCarry, m_faceLeft);
				else
					playAnimation(Animation::Idle, m_faceLeft);
			}
			break;

		case EnemyState::Attack:
			playAnimation(Animation::Punch, m_faceLeft);
			break;

		case EnemyState::PrepareShoot:
			playAnimation(Animation::PrepareKick, m_faceLeft);
			break;

		case EnemyState::Kick:
			playAnimation(Animation::Kick, m_faceLeft);
			break;

		case EnemyState::Die:
			if (m_killCause == KillCause::Electricity)
			{
				playAnimation(Animation::Electricity, m_faceLeft);
			}
			else
			{
				playAnimation(Animation::Die, m_faceLeft);
			}
			break;
	}

	if (isCarrying())
	{
		updatePickedEntityPos();
	}
}