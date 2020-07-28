#include "Precomp.h"
#include "Goal.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Entity/Player/Player.h"
#include "Entity/Ball/Ball.h"
#include "Entity/Shit/Shit.h"
#include "Physics/AABB.h"

using namespace sf;

//--------------------------------------------------------------------------
Goal::Goal(GoalType _goalType) :
	Entity("Goal", Game::get().m_objectTiles),
	m_goalType(_goalType)
{
	SuperVimontBros::get().m_goals.add(this);
	m_isColliderForOtherActors = false;
	Super::updateAABB();
}

//--------------------------------------------------------------------------
Goal::~Goal()
{
	SuperVimontBros::get().m_goals.remove(this);
}

//--------------------------------------------------------------------------
void Goal::updateAABB()
{
	Super::updateAABB();
}

//--------------------------------------------------------------------------
GoalSide Goal::getTerrainSide(const Entity * _entity)
{
	SuperVimontBros & game = SuperVimontBros::get();

	const float y = _entity->getPosition().y;
	const float limit = 15.5f * game.m_levelTiles.m_size.y; //  game.m_level.count().y * game.m_levelTiles.m_size.y / 2.0f;
	if (y > limit)
	{
		return GoalSide::Home;
	}
	else
	{
		return GoalSide::Away;
	}
}

//--------------------------------------------------------------------------
TerrainTestFlags Goal::isInsideField(const Entity * _entity)
{
	SuperVimontBros & game = SuperVimontBros::get();
	TerrainTestFlags test = TerrainTestFlags::None;

	for (uint g = 0; g < game.m_goals.size(); ++g)
	{
		const auto * goal = game.m_goals[g];
		switch (goal->getGoalType())
		{
		case GoalType::Football:
			if (_entity->m_collisionAABB.intersects(goal->getHalfField()))
			{
				if (goal->getGoalSide() == GoalSide::Away)
				{
					test |= TerrainTestFlags::IsInFootballUpHalf;
				}
				else if (goal->getGoalSide() == GoalSide::Home)
				{
					test |= TerrainTestFlags::IsInFootballDownHalf;
				}
			}
			if (_entity->m_collisionAABB.intersects(goal->getGoalSurface()))
			{
				if (goal->getGoalSide() == GoalSide::Away)
				{
					test |= TerrainTestFlags::IsInFootballUpSurface;
				}
				else if (goal->getGoalSide() == GoalSide::Home)
				{
					test |= TerrainTestFlags::IsInFootballDownSurface;
				}
			}
			break;

		case GoalType::Rugby:
			if (_entity->m_collisionAABB.intersects(goal->getHalfField()))
			{
				if (goal->getGoalSide() == GoalSide::Away)
				{
					test |= TerrainTestFlags::IsInRugbyUpHalf;
				}
				else if (goal->getGoalSide() == GoalSide::Home)
				{
					test |= TerrainTestFlags::IsInRugbyDownHalf;
				}
			}
			if (_entity->m_collisionAABB.intersects(goal->getGoalSurface()))
			{
				if (goal->getGoalSide() == GoalSide::Away)
				{
					test |= TerrainTestFlags::IsInRugbyUpGoalArea;
				}
				else if (goal->getGoalSide() == GoalSide::Home)
				{
					test |= TerrainTestFlags::IsInRugbyDownGoalArea;
				}
			}
			break;
		}
	}

	return test;
}

//--------------------------------------------------------------------------
void Goal::init()
{
	Entity::init();
	m_goalSide = getTerrainSide(this);
}

//--------------------------------------------------------------------------
void Goal::update(const float _dt)
{
	Entity::update(_dt);

	SuperVimontBros & game = SuperVimontBros::get();

	const TileInfo & tile = getTileInfo();
	const Vector2f & goalPos = getPosition();

	for (uint i = 0; i < game.m_balls.size(); ++i)
	{
		Ball * ball = game.m_balls[i];
		checkGoal(ball, goalPos, tile, GoalScoredType::Ball);
	}

	const auto & shits = game.m_shits;
	for (uint i = 0; i < shits.size(); ++i)
	{
		Shit * shit = shits[i];

		if (shit->isBallShit())
		{
			checkGoal(shit, goalPos, tile, GoalScoredType::Shit);
		}
	}

	addDebugText("#%u (%s)\n", getGUID(), m_goalSide == GoalSide::Home ? "Home" : "Away");
}

//--------------------------------------------------------------------------
void Goal::draw(sf::RenderTexture & _surface)
{
	Entity::draw(_surface);

	if (Game::get().m_debugDisplay)
	{
		m_blastZone.draw(_surface, { 255,0,0,255 });
		m_halfField.draw(_surface, { 0,0,255,255 });

		const TileInfo & tile = getTileInfo();
		const Vector2f & goalPos = getPosition();

		AABB goalArea(goalPos + Vector2f(m_offsetX, -m_goalHeight / 2.0f), { (float)tile.m_box.width / 2.0f, m_goalHeight / 2.0f });
		goalArea.draw(_surface, { 0,255,0,255 });
	}
}