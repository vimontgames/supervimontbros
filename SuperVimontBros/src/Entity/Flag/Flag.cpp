#include "Precomp.h"
#include "Flag.h"
#include "SuperVimontBros/SuperVimontBros.h"

//--------------------------------------------------------------------------
Flag::Flag(FlagType _flagType) :
	Super("Flag", SuperVimontBros::get().m_objectTiles),
	m_flagType(_flagType)
{

}

//--------------------------------------------------------------------------
void Flag::init()
{
	AnimationSequence & idle = getAnimationSequence(Animation::Idle);

	switch (m_flagType)
	{
		case FlagType::YellowRed:
		{
			idle.addFrame(AnimFrame({ 1,2 }, 300));
			idle.addFrame(AnimFrame({ 2,2 }, 300));
			idle.addFrame(AnimFrame({ 3,2 }, 300));
			idle.addFrame(AnimFrame({ 4,2 }, 300));
		}
		break;

		case FlagType::France:
		{
			idle.addFrame(AnimFrame({ 6,2 }, 300));
			idle.addFrame(AnimFrame({ 7,2 }, 300));
			idle.addFrame(AnimFrame({ 8,2 }, 300));
			idle.addFrame(AnimFrame({ 9,2 }, 300));
		}
		break;
	}

	playAnimation(Animation::Idle);
}