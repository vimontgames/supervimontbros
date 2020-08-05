#pragma once

#include "Visual/Actor/Actor.h"

class Tiles;
class Level;
class Viewport;

typedef u16 EntityGUID;

enum class Animation : AnimIndex
{
	Icon = 0,
	Idle,
	Walk,
	Run,
	Shit,
	Punch,
	PrepareKick,
	Kick,
	Die,
	CelebrateGoal,
	IdleCarry,
	Carry,
	Electricity,
	ThrowUp,
	Fire,

	Count
};

enum class SoundFX : SoundIndex
{
	// Player & Enemy
	GrrrA = 0,
	GrrrB,
	Hit,
	Damage,
	Prout,
	CelebrateGoal,
	CelebrateTry,
	CelebrateDrop,
	Miam,
	MiamPizza,
	MiamPuree,
	Beurk, 
	Fire,

	// Vehicle
	Open = 0,
	Engine = 1,
	Brake = 2,
	Crash = 3
};

class Entity : public Actor
{
public:
	Entity(const sf::String & _name, const Tiles & _tileSet);
	~Entity();

	void updateAABB() override;

	AnimationSequence & getAnimationSequence(Animation _animation);
	bool playAnimation(Animation _animation, bool _left = false, bool _restart = false);
	bool isAnimationPlaying(Animation _animation) const;

	void onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical) override;

	bool addSoundFX(SoundFX _soundFX, const char * _fullpath);
	bool addCustomSoundFX(SoundFX _soundFX, const char * _path, const char * _filename);
	bool playSound(SoundFX _soundFX, int _volume = 50, bool _loop = false);

	bool clampPositionToLevel(const Level & _level);
	bool clampPositionToViewport(const Viewport & _viewport);

	std::vector<Entity*> findEntitiesInRadius(const float _lookupDist);

	bool isPickable() const;
	void setIsPickable(bool _pickable);

	bool isPicked() const;
	void setPicked(bool _isPicked);

	bool pickEntity(Entity * _picked);
	const Entity * getCurrentlyPickedEntity() const;
	bool dropPickedEntity();
	bool updatePickedEntityPos();
	bool isCarrying() const;

	const AABB * getLastDropZoneAABB() const;
	bool setLastDropZoneAABB(AABB * _dropZoneAABB);

	EntityGUID getGUID() const;

private:
	typedef Actor Super;
	static EntityGUID s_nextGUID;

	EntityGUID m_GUID;
	Entity * m_pickedEntity = nullptr;
	AABB * m_lastDropZone = nullptr;
	bool m_isPickable = false;
	bool m_isPicked = false;
};

#ifdef ENABLE_INL
#include "Entity.inl"
#endif