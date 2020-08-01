#pragma once

#include "Core/Core.h"

#define OBJECT_TILEMAP_WIDTH 20
#define OBJECT_TILEMAP_HEIGHT 16
#define OBJECT_INDEX(x,y) (x+y*OBJECT_TILEMAP_WIDTH)
#define OBJECT_COORDS(i) {uint(i) % OBJECT_TILEMAP_WIDTH, uint(i) / OBJECT_TILEMAP_WIDTH}

typedef u16 objectIndex;

enum class LevelObject : objectIndex
{
	Empty				= 0,

	Plot				= OBJECT_INDEX( 1, 0),
	Tomb				= OBJECT_INDEX( 2, 0),
	Crunch				= OBJECT_INDEX( 3, 0),
	Shit				= OBJECT_INDEX( 4, 0),
	CelebrateGoal		= OBJECT_INDEX( 5, 0),
	TrashGreen			= OBJECT_INDEX(10, 0),
	TrashYellow			= OBJECT_INDEX(11, 0),
	TrashGreen2			= OBJECT_INDEX(12, 0),
	BluePill			= OBJECT_INDEX(13, 0),
	RedPill				= OBJECT_INDEX(14, 0),
	GreenPill			= OBJECT_INDEX(15, 0),

	IceCream			= OBJECT_INDEX( 0, 1),
	Pizza				= OBJECT_INDEX( 1, 1),
	Coca				= OBJECT_INDEX( 2, 1),
	Water				= OBJECT_INDEX( 3, 1),
	Hotdog				= OBJECT_INDEX( 4, 1),
	Bone				= OBJECT_INDEX(11, 1),
	ShitFootball		= OBJECT_INDEX(12, 1),
	Chicken				= OBJECT_INDEX(13, 1),
	ShitRugby			= OBJECT_INDEX(14, 1),
	Wine				= OBJECT_INDEX(15, 1),
	Apple				= OBJECT_INDEX(17, 6),

	DogJune				= OBJECT_INDEX( 0, 2),
	Flag				= OBJECT_INDEX( 1, 2),
	Eclair				= OBJECT_INDEX( 5, 2),
	FrenchFlag			= OBJECT_INDEX( 6, 2),
	Stop				= OBJECT_INDEX(10, 2),
	DogLucky			= OBJECT_INDEX(11, 2),
	ChickenBone			= OBJECT_INDEX(13, 2),
	PureeSausage		= OBJECT_INDEX(14, 2),

	Barrier_First		= OBJECT_INDEX(0, 3),
	Barrier_Last		= OBJECT_INDEX(9, 3),

	FootballWhite		= OBJECT_INDEX(0, 4),
	FootballYellow		= OBJECT_INDEX(1, 4),
	BeachBall			= OBJECT_INDEX(2, 4),
	RugbyBall			= OBJECT_INDEX(3, 4),
	RugbyBallWhite		= OBJECT_INDEX(4, 4),
	TennisBall			= OBJECT_INDEX(5, 4),
	ZombieHead			= OBJECT_INDEX(6, 4),
	ZombieHeadSkeleton	= OBJECT_INDEX(7, 4),
	ShitElectric		= OBJECT_INDEX(11, 4),
	ShitElectric1		= OBJECT_INDEX(12, 4),
	Praline				= OBJECT_INDEX(13, 4),

	Zombie				= OBJECT_INDEX(0, 5),
	Footballer			= OBJECT_INDEX(1, 5),
	Rugbyman			= OBJECT_INDEX(2, 5),
	YellowVest			= OBJECT_INDEX(3, 5),
	Footballer2			= OBJECT_INDEX(4, 5),
	Goalkeeper			= OBJECT_INDEX(5, 5), 
	Goalkeeper2			= OBJECT_INDEX(6, 5), 
	ZombiePlaya			= OBJECT_INDEX(7, 5),
	ZombieCowboy		= OBJECT_INDEX(8, 5),

	House0_First		= OBJECT_INDEX(0, 6),
	House0_Last			= OBJECT_INDEX(8, 6),

	House1_First		= OBJECT_INDEX(0, 7),
	House1_Last			= OBJECT_INDEX(8, 7),

	RugbyGoal			= OBJECT_INDEX(12, 9),

	Peugeot3008			= OBJECT_INDEX( 1, 10),
	RenaultClio			= OBJECT_INDEX( 5, 10),
	Peugeot206			= OBJECT_INDEX( 9, 10),

	SandCastle			= OBJECT_INDEX(0, 14),
	SandCastle_Broken	= OBJECT_INDEX(1, 14),
	ElectricBox			= OBJECT_INDEX(14, 14),

	ZombieSplash		= OBJECT_INDEX(0, 15),
	FootballerSplash	= OBJECT_INDEX(1, 15),
	RugbymanSplash		= OBJECT_INDEX(2, 15),
	YellowVestSplash	= OBJECT_INDEX(3, 15),
	Footballer2Splash	= OBJECT_INDEX(4, 15),
	GoalkeeperSplash	= OBJECT_INDEX(5, 15),
	Goalkeeper2Splash	= OBJECT_INDEX(6, 15),
	ZombiePlayaSplash	= OBJECT_INDEX(7, 15),
	ZombieCowboySplash	= OBJECT_INDEX(10, 15),

	CinderSplash		= OBJECT_INDEX(8, 15),
	PlayerThrowUpSplash = OBJECT_INDEX(9, 15),
};