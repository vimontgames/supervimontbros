#include "precomp.h"
#include "ElectricBox.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Tiles/LevelObject.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"

//--------------------------------------------------------------------------
ElectricBox::ElectricBox(bool _open) :
	Super("ElectricBox", SuperVimontBros::get().m_objectTiles),
	m_open(_open)
{

}

//--------------------------------------------------------------------------
void ElectricBox::init()
{
	sf::Vector2u coords = OBJECT_COORDS(LevelObject::ElectricBox);

	AnimationSequence & idle = getAnimationSequence(Animation::Idle);
	idle.addFrame(AnimFrame(coords, 300));

	AnimationSequence & run = getAnimationSequence(Animation::Run);
	run.addFrame(AnimFrame({ coords.x + 1, coords.y }, 1000));
	run.addFrame(AnimFrame({ coords.x + 2, coords.y }, 100));
	run.addFrame(AnimFrame({ coords.x + 1, coords.y }, 200));
	run.addFrame(AnimFrame({ coords.x + 1, coords.y }, 1500));
	run.addFrame(AnimFrame({ coords.x + 3, coords.y }, 100));
	run.addFrame(AnimFrame({ coords.x + 1, coords.y }, 200));
	run.addFrame(AnimFrame({ coords.x + 1, coords.y }, 2000));
	run.addFrame(AnimFrame({ coords.x + 3, coords.y }, 100));
	run.addFrame(AnimFrame({ coords.x + 1, coords.y }, 200));
	run.addFrame(AnimFrame({ coords.x + 2, coords.y }, 150));
	run.addFrame(AnimFrame({ coords.x + 1, coords.y }, 1000));

	if (m_open)
		open(nullptr);
	else
		close();

	addCustomSoundFX(SoundFX::Hit, "SuperVimontBros/data/sound", "Open.wav");
	addCustomSoundFX(SoundFX::Damage, "SuperVimontBros/data/sound", "Bzzz.wav");
}

//--------------------------------------------------------------------------
void ElectricBox::open(Entity * _openedBy)
{
	m_open = true;
	setParent(_openedBy);
	playAnimation(Animation::Run);
	playSound(SoundFX::Hit, 25);
}

//--------------------------------------------------------------------------
void ElectricBox::close()
{
	m_open = false;
	playAnimation(Animation::Idle);
	playSound(SoundFX::Hit, 25);
}

//--------------------------------------------------------------------------
void ElectricBox::onBeingCollided(Actor * _byActor, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	if (m_open)
	{
		Enemy * enemy = dynamic_cast<Enemy*>(_byActor);
		if (enemy)
		{
			if (enemy->kill(dynamic_cast<Player*>(getParent()), KillCause::Electricity))
				playSound(SoundFX::Damage, 50);
		}

		Player * player = dynamic_cast<Player*>(_byActor);
		if (player)
		{
			if (player->onElectricityHit(this))
				playSound(SoundFX::Damage, 75);
		}
	}

	Super::onBeingCollided(_byActor, _move, _horizontal, _vertical);
}