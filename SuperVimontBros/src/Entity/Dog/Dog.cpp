#include "Precomp.h"
#include "Dog.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Entity/Shit/Shit.h"
#include "Entity/Player/Player.h"

using namespace sf;

//--------------------------------------------------------------------------
PlayerType dogTypeToPlayerType(DogType _dogType)
{
	switch (_dogType)
	{
		default:
			assert(false);

		case DogType::Lucky:
			return PlayerType::Lucky;

		case DogType::June:
			return PlayerType::June;
	}
}

//--------------------------------------------------------------------------
const char * Dog::getDogName(DogType _dogType) const
{
	return PlayerTypeInfo::get(dogTypeToPlayerType(_dogType)).name;
}

//--------------------------------------------------------------------------
Dog::Dog(DogType _dogType) :
	Entity(getDogName(_dogType), Game::get().m_spritesTile),
	m_dogType(_dogType)
{
	m_hasAnimLeft = false;
	m_isCollidingWithParent = true;
}

//--------------------------------------------------------------------------
Dog::~Dog()
{

}

//--------------------------------------------------------------------------
void Dog::init()
{
	Super::init();

	uint line = DogType::June == m_dogType ? 8 : 9;

	AnimationSequence & idle = getAnimationSequence(Animation::Idle);
					    idle.addFrame(AnimFrame({ 11, line }, 1000));

	AnimationSequence & shit = getAnimationSequence(Animation::Shit);
						shit.addFrame(AnimFrame({ 13,line }, 1000));
						shit.addFrame(AnimFrame({ 14,line }, 2000));

	AnimationSequence & walk = getAnimationSequence(Animation::Walk);
						walk.addFrame(AnimFrame({ 1,line }, 250));
						walk.addFrame(AnimFrame({ 2,line }, 250));
						walk.addFrame(AnimFrame({ 3,line }, 250));
						walk.addFrame(AnimFrame({ 4,line }, 250));

	playAnimation(Animation::Idle);

	addCustomSoundFX(SoundFX::CelebrateGoal, "SuperVimontBros/data/sound", "Woof.wav");
	addCustomSoundFX(SoundFX::Prout, "SuperVimontBros/data/sound", "prout.wav");

	switch (m_dogType)
	{
		case DogType::Lucky:
			m_wakeUpTime = 3.0f;
			m_speed = 0.04f;
			m_shitTime = 15.0f;
			break;

		case DogType::June:
			m_wakeUpTime = 2.0f;
			m_speed = 0.035f;
			m_shitTime = 5.0f;
			break;
	}
}

//--------------------------------------------------------------------------
void Dog::update(const float _dt)
{
	Super::update(_dt);

	SuperVimontBros & game = SuperVimontBros::get();

	for (auto & player : game.m_players)
	{
		switch (m_dogType)
		{
			case DogType::June:
				if (player->getPlayerType() == PlayerType::June)
				{
					Game::get().ReleaseAsync(this);
					return;
				}
				break;

			case DogType::Lucky:
				if (player->getPlayerType() == PlayerType::Lucky)
				{
					Game::get().ReleaseAsync(this);
					return;
				}
				break;

			default:
				break;
		}
		
	}

	bool walking = false;

	bool shiting = false;
	if (!m_shitting)
	{
		if (m_walking)
		{
			if (m_shitTimer.getElapsedTime().asSeconds() > m_shitTime)
			{
				shiting = true;
				m_shitting = true;
				playSound(SoundFX::Prout);
				m_shitTimer.restart();

				Shit * shit = new Shit();
				int shitOffsetX = (m_faceLeft ? +3 + 5 : -1 - 5);
				shit->init();
				shit->setPosition(getPosition().x + shitOffsetX, getPosition().y,+4);
				shit->setParent(this->getParent());
				shit->setColor(PlayerTypeInfo::get(dogTypeToPlayerType(m_dogType)).shitColor);
				Game::get().registerVisual(shit);
				shit->release();
			}
		}
	}
	else
	{
		if (m_shitTimer.getElapsedTime().asSeconds() > m_wakeUpTime)
		{
			m_shitting = false;
			shiting = false;
			m_shitTimer.restart();
		}
	}

	if (getParent() != nullptr && !shiting)
	{
		Vector2f dir = getParent()->getPosition() - getPosition();
		
		float n = length(dir);

		m_faceLeft = dir.x < 0.0f;

		const auto s = m_walkTimer.getElapsedTime().asSeconds();

		if (n > 16.0f && s > m_wakeUpTime)
		{
			dir /= n;

			float speed = m_speed;

			Vector2f m = dir *_dt;
					 m.x *= speed;
					 m.y *= speed * 0.5f;

			if (game.tryMove(this, m))
			{
				move(m);
				walking = true;
			}
		}

		if (m_faceLeft)
		{
			m_animLeft = true;
		}
		else
		{
			m_animLeft = false;
		}
	}

	if (walking)
	{
		if (!m_walking)
		{
			playSound(SoundFX::CelebrateGoal);
			m_walking = true;
			m_shitTimer.restart();
		}
	}
	else
	{
		if (m_walking)
		{
			m_walkTimer.restart();
			m_walking = false;
		}
	}

	if (m_shitting)
	{
		playAnimation(Animation::Shit, m_animLeft);
	}
	else if (walking)
	{
		playAnimation(Animation::Walk, m_animLeft);
	}
	else
	{
		playAnimation(Animation::Idle, m_animLeft);
	}

	addDebugText("Walk %u\nShit %u", m_walking ? 1 : 0, m_shitting ? 1 : 0);
}

//--------------------------------------------------------------------------
void Dog::onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	Shit * shit = dynamic_cast<Shit*>(_other);
	if (shit)
	{
		return;
	}

	Super::onActorCollision(_other, _move, _horizontal, _vertical);
}