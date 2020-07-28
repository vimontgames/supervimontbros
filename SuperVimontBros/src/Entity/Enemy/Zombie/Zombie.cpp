#include "Precomp.h"
#include "Zombie.h"
#include "Game/Game.h"
#include "Tiles/SpriteModel.h"

using namespace sf;

//--------------------------------------------------------------------------
Zombie::Zombie() :
	Enemy("Zombie", Game::get().m_spritesTile, SPRITE_LINE(SpriteModel::Zombie))
{

}

//--------------------------------------------------------------------------
Zombie::~Zombie()
{

}

//--------------------------------------------------------------------------
void Zombie::init()
{
	Enemy::init();
}