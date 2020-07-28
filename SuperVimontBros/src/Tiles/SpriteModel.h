#pragma once

#include "Core/Core.h"

#define SPRITE_TILEMAP_WIDTH 32
#define SPRITE_TILEMAP_HEIGHT 32
#define SPRITE_INDEX(x,y) (x+y*SPRITE_TILEMAP_WIDTH)

#define SPRITE_COLUMN(index) (((uint)index) % SPRITE_TILEMAP_WIDTH)
#define SPRITE_LINE(index) (((uint)index)/SPRITE_TILEMAP_WIDTH)

typedef u16 spriteIndex;
enum class SpriteModel : spriteIndex
{
	Romeo			= SPRITE_INDEX(0, 0),
	Pablo			= SPRITE_INDEX(0, 2),
	Benualdo		= SPRITE_INDEX(0, 6),
	Marie			= SPRITE_INDEX(0,16),
	June			= SPRITE_INDEX(0, 8),
	Lucky			= SPRITE_INDEX(0, 9),
	Praline			= SPRITE_INDEX(0, 20),
	Yvan			= SPRITE_INDEX(0, 21),

	ZombiePlaya		= SPRITE_INDEX(0, 3),
	Zombie			= SPRITE_INDEX(0, 4),
	YellowVest		= SPRITE_INDEX(0, 5),
	Footballer		= SPRITE_INDEX(0,10),
	Rugbyman		= SPRITE_INDEX(0,12),
	Footballer2		= SPRITE_INDEX(0,17),
	Goalkeeper		= SPRITE_INDEX(0,18),
	Goalkeeper2		= SPRITE_INDEX(0,19),
	ZombieCowboy	= SPRITE_INDEX(0, 22),
};