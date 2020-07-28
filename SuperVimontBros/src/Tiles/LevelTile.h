#pragma once

#include "Core/Core.h"

#define BACKGROUND_TILEMAP_WIDTH 20
#define BACKGROUND_TILEMAP_HEIGHT 16
#define BACKGROUND_INDEX(x,y) (x+y*BACKGROUND_TILEMAP_WIDTH)

typedef u16 objectIndex;

enum class LevelTile : objectIndex
{
	ZombieHole = BACKGROUND_INDEX(1, 1)
};