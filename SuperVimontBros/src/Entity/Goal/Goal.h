#pragma once

#include "Entity/Entity.h"

enum class GoalScoredType : u8
{
	Ball = 0,
	Shit,

	Count
};

enum class GoalType : u8
{
	Football = 0,
	Rugby,

	Count
};

enum class GoalSide : u8
{
	Home = 0,	// down midfield, scores in upper goal
	Away,		// up midfield, scores in downer goal

	Count,
	Unknown = Count
};

enum class TerrainTestFlags : u16
{
	None = 0x0000,

	IsInFootballUpHalf = 0x0001,
	IsInFootballDownHalf = 0x0002,
	IsInFootballUpSurface = 0x0004,
	IsInFootballDownSurface = 0x0008,

	IsInFootballField = IsInFootballUpHalf | IsInFootballDownHalf,
	IsInFootballSurface = IsInFootballUpSurface | IsInFootballDownSurface,

	IsInRugbyUpHalf = 0x0010,
	IsInRugbyDownHalf = 0x0020,
	IsInRugbyUpGoalArea = 0x0040,
	IsInRugbyDownGoalArea = 0x0080,

	IsInRugbyField = IsInRugbyUpHalf | IsInRugbyDownHalf,
	IsInRugbyGoalArea = IsInRugbyUpGoalArea | IsInRugbyDownGoalArea
};
ENUM_FLAGS_OPERATORS(TerrainTestFlags);

class Goal : public Entity
{
public:
	Goal(GoalType _goalType);
	~Goal();

	void init() override;
	void update(const float _dt) override;
	void draw(sf::RenderTexture & _surface) override;

	void updateAABB();

	const AABB & getGoalSurface() const { return m_blastZone; }
	const AABB & getHalfField() const { return m_halfField; }

	GoalType getGoalType() const { return m_goalType; }
	GoalSide getGoalSide() const { return m_goalSide; }

	static GoalSide getTerrainSide(const Entity * _entity);
	static TerrainTestFlags isInsideField(const Entity * _entity);

//protected:
	virtual void checkGoal(Entity * _entity, const sf::Vector2f & _goalPos, const TileInfo & _tile, GoalScoredType _type) = 0;

//protected:
	GoalType m_goalType;
	GoalSide m_goalSide;

	float m_goalHeight;
	float m_offsetX = 0.0f;

	AABB m_blastZone;
	AABB m_halfField;

	using Super = Entity;
};