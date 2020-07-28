#include "Precomp.h"
#include "Peugeot3008.h"
#include "SuperVimontBros/SuperVimontBros.h"

using namespace sf;

//--------------------------------------------------------------------------
Peugeot3008::Peugeot3008() :
	Vehicle("Peugeot3008")
{
	m_baseSpeedForward = 1.75f;
	m_baseSpeedBackward = 1.5f;
	m_turboSpeedForward = 4.5f;
	m_turboSpeedBackward = 2.5f;

	m_maxSpeedForward = 0.75f;
	m_maxSpeedBackward = 0.50f;

	m_friction = 1.0f / 3000.0f;
	m_bounce = 0.3f;
	m_steering = 0.2f;
}

//--------------------------------------------------------------------------
Peugeot3008::~Peugeot3008()
{

}

//--------------------------------------------------------------------------
void Peugeot3008::init()
{
	Vehicle::init();

	AnimationSequence & idle = getAnimationSequence(Animation::Idle);
	idle.addFrame(AnimFrame({ 1, 10 }));

	Vector2u imgWheel = { 5,14 };

	const Vector2f wheelPos[2] =
	{
		{ -32.0f,-5.0f  },
		{ +41.0f,-5.0f  }
	};

	if (m_faceLeft)
	{
		addWheel(imgWheel, { wheelPos[0].x - 10.0f, wheelPos[0].y });
		addWheel(imgWheel, { wheelPos[1].x - 10.0f, wheelPos[1].y });
	}
	else
	{
		addWheel(imgWheel, { wheelPos[0].x, wheelPos[0].y });
		addWheel(imgWheel, { wheelPos[1].x, wheelPos[1].y });
	}	

	playAnimation(Animation::Idle, m_faceLeft);
}

//--------------------------------------------------------------------------
void Peugeot3008::updateAABB()
{
	Super::updateAABB();

	if (m_faceLeft)
	{
		m_collisionAABB.m_pos.x -= 14.0f;
		m_visibilityAABB.m_pos.x -= 14.0f;
	}
}