#include "Precomp.h"
#include "Marseillais.h"
#include "Game/Game.h"
#include "Tiles/SpriteModel.h"

using namespace sf;

//--------------------------------------------------------------------------
Footballer::Footballer() :
	Enemy("Marseillais", Game::get().m_spritesTile, SPRITE_LINE(SpriteModel::Footballer))
{
	m_chasePlayers = true;
	m_chaseBalls = true;
	
	m_speed = 0.025f;
	m_waitBeforeWalk = 128;
	m_walkMaxDist = 256;
}

//--------------------------------------------------------------------------
Footballer::~Footballer()
{

}

//--------------------------------------------------------------------------
void Footballer::init()
{
	Enemy::init();
}