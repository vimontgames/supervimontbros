#pragma once

#include "Core/Core.h"

#define DECAL_TILEMAP_WIDTH 20
#define DECAL_TILEMAP_HEIGHT 16
#define DECAL_INDEX(x,y) (x+y*DECAL_TILEMAP_WIDTH)

typedef u16 decalIndex;

enum class DecalTile : decalIndex
{
	
};