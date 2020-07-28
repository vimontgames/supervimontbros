#include "Precomp.h"
#include "Breakable.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Tiles/LevelObject.h"
#include "Entity/Player/Player.h"

//--------------------------------------------------------------------------
Breakable::Breakable(BreakableType _breakableType) :
	Super("Breakable", SuperVimontBros::get().m_objectTiles),
	m_breakableType(_breakableType)
{
	setImage(OBJECT_COORDS(LevelObject::SandCastle));
}

//--------------------------------------------------------------------------
void Breakable::init()
{
	Super::init();
}

//--------------------------------------------------------------------------
void Breakable::onBeingCollided(Actor * _byActor, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	if (!m_broken)
	{
		setImage(OBJECT_COORDS(LevelObject::SandCastle_Broken));
		m_isColliderForOtherActors = false;

		m_broken = true;

		Player * player = dynamic_cast<Player*>(_byActor);
		if (player)
			player->addScore(1);
	}
}