#pragma once

#include "Entity/Entity.h"
#include "Controller/Controller.h"
#include "Entity/Goal/Goal.h"

class Goal;
class Vehicle;
class Viewport;
class Enemy;
class Bonus;
class Shit;
class Bullet;

enum class PlayerType : u8
{
    Romeo = 0,
    Pablo,
    Benualdo,
    Marie,
    Yvan,
    June,
    Lucky,
    Praline,
    Benjamin,

	Count
};

struct PlayerTypeInfo
{
	static const PlayerTypeInfo & get(PlayerType _playerType);
	#define defaultShitColor { 145, 100, 41 }

	enum Flags : u32
	{
		None		  = 0x00000000,

		AnimLeft	  = 0x00000001,
		ShitFootBall  = 0x00000002,
		IsADog		  = 0x00000004,
		StrongStomach = 0x00000008,
		AtomicFart	  = 0x00000010,
		ShitRugby	  = 0x00000020,
		CanCarry	  = 0x00000040,
		CanDrinkWine  = 0x00000080,
		ShitElectric  = 0x00000100,
	};

	PlayerTypeInfo(const char * _name, u8 _spriteLine, sf::Color _shitColor = defaultShitColor, Flags _flags = (Flags)0, u8 _pal0 = 0, u8 _pal1 = 0, u8 _pal2 = 0, u8 _pal3 = 0) :
		name(_name),
		line(_spriteLine),
		shitColor(_shitColor),
		flags(_flags)
	{
		palette[0] = _pal0;
		palette[1] = _pal1;
		palette[2] = _pal2;
		palette[3] = _pal3;
	}

	const char * name;
	u8 line;
	sf::Color shitColor;
	Flags flags;
	u8 palette[4];
};

ENUM_FLAGS_OPERATORS(PlayerTypeInfo::Flags);

enum class PlayerDamageType : u8
{
	Zombie,
	Rotten,
	Electricity,
	Bullet,

	Count
};

class Player : public Entity
{
public:
	enum class Button
	{
		Shit = 0,
		Run = 1,
		Kick = 2,
		Punch = 3,

		Count
	};

							Player	(ControllerID _controllerID, PlayerType _playerType, u8 _playerTypeIndex = 0);

	PlayerType				getPlayerType() const;
	u8						getPlayerTypeSubIndex() const;

	void					init	();
	void					setViewport(Viewport & _viewport);

	void					activate();
	void					deactivate();

	void					update	(const float _dt) override;
	void					onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;

	void					onEnemyHit(Enemy * _enemy);
	bool					onBonusHit(Bonus * _bonus, bool _share = false);
	bool					onElectricityHit(Entity * _byEntity);
	void					onShitHit(Shit * _shit);
	bool					onBulletHit(Bullet * _bullet);

	void					onEnterVehicle(Vehicle * _vehicle);
	void					onExitVehicle();

	bool					canTakeDamage() const;

	bool					setRotten(bool _isRotten);

	void					draw	(sf::RenderTexture & _window) override;
	void					drawShadow(sf::RenderTexture & _surface) override;

	uint					getScore	() const;
	uint					addScore	(i16 _points);

	uint					getZombieKilled() const;
	uint					addZombieKilled(i16 _count);

	uint					getGoalScored(GoalType _goalType) const;
	uint					addGoalScored(const Goal * _goal, i16 _points, float _distance, GoalType _goalType, GoalSide _goalSide);
	void					celebrate();

	bool					isActive() const { return m_isActive; }
	bool					isInVehicle() const { return m_vehicle; }

	const Viewport *		getViewport() const { return m_viewport; }
	Viewport *				getViewport() { return m_viewport; }

	inline const Controller & getController() const;

	static void				setupPlayerAnimations(Entity * _entity, PlayerType _playerType);

	bool					AddRemoveHP(int _hp);

private:
	using Super = Entity;

	ControllerID			m_controllerID;
	PlayerType				m_playerType;
	u8						m_playerTypeIndex;
	Viewport *				m_viewport = nullptr;
	bool					m_shitDone = false;
	bool					m_kickDone = false;
	bool					m_hasEverTakenDamage = false;
	const Entity *			m_entityToDropKick = nullptr;
	u16						m_score = 0;
	u16						m_zombieKilled = 0;
	u16						m_goalScored[(uint)GoalType::Count];
	bool					m_celebrate = false;
	bool					m_isActive = false;
	Vehicle *				m_vehicle = nullptr;
	sf::Clock				m_timeSinceLastHit;
	bool					m_isAttacked = false;
	bool					m_damageTaken = false;
	i32						m_damagePending = 0;
	PlayerDamageType		m_damageType = PlayerDamageType::Zombie;
	bool					m_isRotten = false;
	sf::Clock				m_rottenTimer;
	uint					m_shitWarmupTime = 1000;
	uint					m_lastHPFade = uint(-1);
};

#ifdef ENABLE_INL
#include "Player.inl"
#endif
