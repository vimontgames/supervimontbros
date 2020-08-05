#pragma once

#include "Entity/Entity.h"
#include <functional>

class Bullet;

enum class GoalSide : u8;
enum class GoalType : u8;

enum class EnemyState
{
	Idle = 0,
	Wait,
	Player,
	Ball,
	Attack,
	Die,
	PrepareShoot,
	Kick,
	Strafe,
	Fire,

	Count
};

enum class EnemyType : u8
{
	Zombie = 0,
	Footballer,
	Footballer2,
	Goalkeeper,
	Goalkeeper2,
	Rugbyman,
	YellowVest,
	ZombiePlaya,
	ZombieCowboy,
	ZombiePatrick,

	Count
};

enum class KillCause : u8
{
	Default = 0,
	Electricity,
	Bullet,

	Count
};

template <class T> struct FindResult
{
	bool found() const { return nullptr != entity; }

	const T * entity = nullptr;
	sf::Vector2f dir = { 0.0f, 0.0f };
	float dist = FLT_MAX;
};

struct EnemyTypeInfo
{
	static const EnemyTypeInfo & get(EnemyType _enemyType);

	enum Flags : u32
	{
		None			= 0x00000000,

		ChasePlayers	= 0x00000001,
		ChaseFootball	= 0x00000002,
		ChaseRugbyBall	= 0x00000004,
		GoalKeeper		= 0x00000008,
		CanFire			= 0x00000010,

		AnimLeft		= 0x10000000
	};

	EnemyTypeInfo(const char * _name, u8 _spriteLine, Flags _flags = (Flags)0, float _speed = 0.02f, uint _waitBeforeWalk = 512, uint _walkMaxDist = 384, uint _distanceToOtherEnemies = 16) :
		name(_name),
		line(_spriteLine),
		flags(_flags),
		m_speed(_speed),
		m_waitBeforeWalk(_waitBeforeWalk),
		m_walkMaxDist(_walkMaxDist),
		m_distanceToOtherEnemies(_distanceToOtherEnemies)
	{

	}

	const char * name;
	u8 line;
	Flags flags;
	float m_speed;
	uint m_waitBeforeWalk;
	uint m_walkMaxDist;
	uint m_distanceToOtherEnemies;

};

ENUM_FLAGS_OPERATORS(EnemyTypeInfo::Flags);

class Shit;
class Ball;

class Enemy : public Entity
{
public:
	Enemy(EnemyType _enemyType);
	~Enemy();

	bool testFlags(EnemyTypeInfo::Flags _flags) const;

	void init() override;
	void update(const float _dt) override;

	void onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical);

	bool onShitHit(Shit * _shit);
	bool onHitBall(Ball * _ball);
	bool onBulletHit(Bullet * _bullet);

	bool kill(Player * _byPlayer, KillCause _cause = KillCause::Default);
	bool setAttacking();

	GoalSide getSide() const { return m_side; }
	EnemyState getEnemyState() const { return m_enemyState; }
	const EnemyTypeInfo & getInfo() const { return EnemyTypeInfo::get(m_enemyType); }

protected:
	void enterState(EnemyState _newState);

	bool updateLookAt(const sf::Vector2f & _moveDir, float _dt);

	template <class T> static FindResult<T> findClosest(const Entity * _entity, const std::vector<T*> & _from);
	template <class T> static FindResult<T> findClosest_if(const Entity * _entity, const std::vector<T*> & _from, std::function<bool(const T *)> _condition);
	template <class T> static FindResult<T> findClosest_alive(const T * _entity, const std::vector<T*> & _from);
	template <class T> static std::vector<T*> findEntitiesInRadius(const std::vector<T*> & _from, float _radius);
	template <class T> static bool isCloserThan(const Entity * _entity, const Entity * _target, const std::vector<T*> & _others, float _radius = FLT_MAX);

	static bool isVisibleInAnyViewport(const Entity * _entity);
	static bool findGoalDir(sf::Vector2f & _dir, Entity * _entity, GoalType _goalType, GoalSide _goalSide);

private:
	static sf::Sound * s_lastGrrSound;

	EnemyType	m_enemyType;
	EnemyState	m_enemyState = EnemyState::Idle;
	KillCause	m_killCause = KillCause::Default;
	GoalSide	m_side;

	sf::Clock m_waitTimer;

private:
	using Super = Entity;
};