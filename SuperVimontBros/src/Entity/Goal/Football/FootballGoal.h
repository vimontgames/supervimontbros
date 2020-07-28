#pragma once

#include "Entity/Goal/Goal.h"

class FootballGoal : public Goal
{
public:
	FootballGoal() :
		Goal(GoalType::Football)
	{

	}

private:
	void updateAABB() override;
	void checkGoal(Entity * _entity, const sf::Vector2f & _goalPos, const TileInfo & _tile, GoalScoredType _type) override;

	using Super = Goal;
};