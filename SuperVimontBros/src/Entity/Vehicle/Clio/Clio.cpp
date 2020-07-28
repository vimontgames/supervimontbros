#include "Precomp.h"
#include "Clio.h"
#include "SuperVimontBros/SuperVimontBros.h"

using namespace sf;

//--------------------------------------------------------------------------
Clio::Clio(ClioType _clioType) :
	Vehicle("Clio"),
	m_clioType(_clioType)
{
	m_baseSpeedForward = 1.5f;
	m_baseSpeedBackward = 1.0f;
	m_turboSpeedForward = 3.0f;
	m_turboSpeedBackward = 1.25f;

	m_maxSpeedForward = 0.5f;
	m_maxSpeedBackward = 0.3f;

	m_friction = 1.0f / 2000.0f;
	m_bounce = 0.4f;
	m_steering = 0.3f;

	switch (m_clioType)
	{
	case ClioType::Blue:
		break;

	case ClioType::Red:
		m_baseSpeedForward = 1.4f;
		m_baseSpeedBackward = 1.1f;
		m_bounce = 0.45f;
		break;
	}
}

//--------------------------------------------------------------------------
Clio::~Clio()
{
}

//--------------------------------------------------------------------------
void Clio::init()
{
	Vehicle::init();

	Vector2u carImg, wheelImg;

	switch (m_clioType)
	{
		case ClioType::Blue:
			carImg = { 5, 10 };
			wheelImg = { 6, 14 };
			break;

		case ClioType::Red:
			carImg = { 9, 10 };
			wheelImg = { 7, 14 };
			break;
	}

	AnimationSequence & idle = getAnimationSequence(Animation::Idle);
	idle.addFrame(AnimFrame(carImg));

	Vector2u imgWheel = wheelImg;

	const Vector2f wheelPos[2] =
	{
		{ -27.0f, -4.0f },
		{ +42.0f, -4.0f }
	};

	if (m_faceLeft)
	{
		addWheel(imgWheel, { wheelPos[0].x - 15.0f, wheelPos[0].y });
		addWheel(imgWheel, { wheelPos[1].x - 15.0f, wheelPos[1].y });
	}
	else
	{
		addWheel(imgWheel, { wheelPos[0].x, wheelPos[0].y });
		addWheel(imgWheel, { wheelPos[1].x, wheelPos[1].y });
	}

	playAnimation(Animation::Idle, m_faceLeft);
}

//--------------------------------------------------------------------------
void Clio::updateAABB()
{
	Super::updateAABB();

	if (m_faceLeft)
	{
		m_collisionAABB.m_pos.x -= 18.0f;
		m_visibilityAABB.m_pos.x -= 18.0f;
	}
}