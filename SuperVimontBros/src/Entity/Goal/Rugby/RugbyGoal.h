#pragma once

#include "Entity/Goal/Goal.h"

class RugbyGoal : public Goal
{
public:
	RugbyGoal() :
		Goal(GoalType::Rugby)
	{

	}

private:
	void updateAABB() override;
	void checkGoal(Entity * _entity, const sf::Vector2f & _goalPos, const TileInfo & _tile, GoalScoredType _type) override;

	using Super = Goal;
};