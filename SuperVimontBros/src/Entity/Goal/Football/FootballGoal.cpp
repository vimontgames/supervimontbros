#include "Precomp.h"
#include "FootballGoal.h"
#include "Entity/Player/Player.h"
#include "SuperVimontBros/SuperVimontBros.h"

using namespace sf;

//--------------------------------------------------------------------------
void FootballGoal::updateAABB()
{
	Super::updateAABB();

	SuperVimontBros & game = SuperVimontBros::get();
	const auto tile = game.m_levelTiles.m_size;

	const auto pos = getPosition();
	Vector2i cellPos = { (int)(pos.x / tile.x), (int)(pos.y / tile.y) };

	Vector2i surface[2];
	Vector2i halfField[2];

	switch(m_goalSide)
	{
		case GoalSide::Away:
			surface[0] = { cellPos.x - 4, cellPos.y };
			surface[1] = { cellPos.x + 4, cellPos.y + 4 };

			halfField[0] = { cellPos.x - 10, cellPos.y };
			halfField[1] = { cellPos.x + 10, cellPos.y + 9 };
			break;

		case GoalSide::Home:
			surface[0] = { cellPos.x - 4, cellPos.y - 4 };
			surface[1] = { cellPos.x + 4, cellPos.y };

			halfField[0] = { cellPos.x - 10, cellPos.y - 9 };
			halfField[1] = { cellPos.x + 10, cellPos.y };
			break;
	}

	m_blastZone = AABB(Vector2f((surface[0].x + 0.5f) * tile.x, (surface[0].y + 0.5f) * tile.y), sf::IntRect(0, 0, tile.x * (surface[1].x - surface[0].x), tile.y * (surface[1].y - surface[0].y)));
	m_halfField = AABB(Vector2f((halfField[0].x + 0.5f) * tile.x, (halfField[0].y + 0.5f) * tile.y), sf::IntRect(0, 0, tile.x * (halfField[1].x - halfField[0].x), tile.y * (halfField[1].y - halfField[0].y)));

	m_goalHeight = 34.0f;
}

//--------------------------------------------------------------------------
void FootballGoal::checkGoal(Entity * _entity, const sf::Vector2f & _goalPos, const TileInfo & _tile, GoalScoredType _type)
{
	auto * parent = _entity->getParent();
	if (nullptr == parent)
		return;

	const Vector2f & pos = _entity->getPosition();
	const Vector2f & prev = _entity->m_previousPos;
	const float height = _entity->m_height;

	if (_entity->m_firstFrame == false && !_entity->isPicked())
	{
		if (pos != prev)
		{
			const bool passed = GoalSide::Home == m_goalSide ? pos.y >= _goalPos.y && prev.y < _goalPos.y : pos.y < _goalPos.y && prev.y >= _goalPos.y;

			if (passed && height <= m_goalHeight)
			{
				if (pos.x >= _goalPos.x + m_offsetX - _tile.m_box.width / 2 && pos.x <= _goalPos.x + m_offsetX + _tile.m_box.width / 2)
				{
					const float dist = length(pos - parent->getPosition());
	
					Player * player = dynamic_cast<Player*>(parent);
					if (player)
					{
						const auto & info = PlayerTypeInfo::get(player->getPlayerType());
						if (_type == GoalScoredType::Ball)
						{
							player->addGoalScored(this, 1, dist, GoalType::Football, m_goalSide);
							player->addScore(dist / 10);
							player->celebrate();
						}
						else if (_type == GoalScoredType::Shit && (PlayerTypeInfo::Flags::ShitFootBall & info.flags))
						{
							player->addGoalScored(this, 1, dist, GoalType::Football, m_goalSide);
							player->addScore(dist / 50);
							player->celebrate();
						}
					}
					else
					{
						SuperVimontBros::get().onGoalScored((Entity*)parent, 1, dist, GoalType::Football, m_goalSide);
					}
				}
				else
				{
					//debugPrint("MISSED 0x%08X!\n", this);
				}
			}
		}
	}
}