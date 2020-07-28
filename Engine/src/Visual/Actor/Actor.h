#pragma once

#include "Visual/Visual.h"
#include "Level/Level.h"
#include <SFML/Audio.hpp>

typedef uint AnimIndex;
static const AnimIndex invalidAnimIndex = (AnimIndex)-1;
static const uint maxAnimIndex = 16;

typedef uint SoundIndex;
static const SoundIndex invalidSoundIndex = (SoundIndex)-1;
static const uint maxSoundIndex = 16;

class Tiles;
class Player;

struct AnimFrame
{
	AnimFrame(const sf::Vector2u & _imageCoords, uint _durationInMs = 1000) :
		m_imageCoords(_imageCoords),
		m_duration(_durationInMs)
	{

	}

	sf::Vector2u m_imageCoords;
	uint m_duration;
};

struct AnimationSequence : std::vector<AnimFrame>
{
	uint addFrame(const AnimFrame & _animFrame)
	{
		const uint count = (uint)size();
		push_back(_animFrame);
		m_totalDuration += _animFrame.m_duration;
		return count;
	}

	uint m_totalDuration = 0;
};

class Actor : public Visual
{
public:
									Actor						(const sf::String & _name, const Tiles & _tileSet);
									~Actor						();

	void							init						() override;
	void							draw						(sf::RenderTexture & _surface) override;
	void							drawShadow					(sf::RenderTexture & _surface) override;

	void							update						(const float _dt) override;

	virtual void					onActorCollision			(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical);
	virtual void					onObjectCollision			(objectIndex _objectIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical);
	virtual void					onTileCollision				(tileIndex _tileIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical);

	virtual void					onBeingCollided				(Actor * _byActor, sf::Vector2f & _move, bool _horizontal, bool _vertical);

	// Animation
	AnimationSequence &				getAnimationSequence		(AnimIndex _animIndex);
	const AnimationSequence &		getAnimationSequence		(AnimIndex _animIndex) const;
	const AnimationSequence &		getCurrentAnimationSequence	() const;
	bool							playAnim					(AnimIndex _animIndex, bool _left = false, bool _restart = false);
	//sf::String						getAnimationName			(AnimIndex _animation) const;

	// Sound
	bool							addSound					(SoundIndex _sound, const char * _fullpath);
	bool							addCustomSound				(SoundIndex _sound, const char * _path, const char * _filename);
	bool							playSound					(SoundIndex _sound, int _volume = 50, bool _loop = false);
	bool							isCurrentSound				(SoundIndex _sound) const;
	bool							isSoundPlaying				(SoundIndex _sound) const;
	sf::String						getSoundName				(SoundIndex _sound);
	void							setSoundVolume				(int _volume);

	// Fade
	void							fadeOutAndKill				(uint _ms);
	bool							isFadedOut					();

	// Movement
	bool							isMoving					() const;

	// Debug test
	void							clearDebugText				();
	void							addDebugText				(const char * _format, ...);

	void							setParent					(Visual * _parent);
	Visual *						getParent					() const;

protected:
	sf::Sound *						getSoundInstance			();

//protected:
public: // TODO clean
	// Logic
	static const uint				s_maxHp = 32;
	uint							m_hp = 4;
	bool							m_faceLeft = false;

	// Animation
	AnimIndex						m_animIndex = invalidAnimIndex;
	uint							m_animFrame = -1;
	bool							m_animLeft = false;
	bool							m_hasAnimLeft = false;
	std::vector<AnimationSequence>	m_animSequences;
	sf::Clock						m_animTimer;

	// Fade
	sf::Clock						m_lifeTime;
	uint							m_fadeStartTime = -1;
	uint							m_fadeDuration = -1;
	bool							m_fading = false;
	bool							m_fadeOut = false; 	
	bool							m_fadeIn = false;
	bool							m_waitRespawn = false;
	bool							m_respawn = false;		// Entity is not killed after fade but will respawn after m_respawnTime
	sf::Clock						m_respawnTimer;
	float							m_respawnTime = 30.0f;

	// Physics
	bool							m_isCollidingWithTiles = true;
	bool							m_isCollidingWithObjects = true;
	bool							m_isCollidingWithOtherActors = true;
	bool							m_isColliderForOtherActors = true;
	bool							m_rotateWhenMoving = false;
	bool							m_isCollidingWithParent = false;

	bool							m_firstFrame = true;
	sf::Vector2f					m_previousPos;
	sf::Vector2f					m_velocity = { 0,0 };
	float							m_jump = 0.0f;
	float							m_kickSpeed = 0.0f;				// 0 = no physics
	float							m_bounce = 0.0f;				// 0 = no bounce 1.0f = max bounce
	float							m_friction = 1.0f/ 1500.0f;		// 0 = infinite run 

	uint							m_collisionKernel = 5;

	// Sound
	int								m_soundIndex[maxSoundIndex];
	sf::Sound *						m_soundInstance = nullptr;
	sf::Clock						m_soundTimer;
	SoundIndex						m_curSoundIndex = invalidSoundIndex;

private:
	// Debug text
	char *							m_debugBuffer = nullptr;
	Visual *						m_parent = nullptr;
};

#ifdef ENABLE_INL
#include "Actor.inl"
#endif