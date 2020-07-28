#include "Precomp.h"
#include "RugbyGoal.h"
#include "Entity/Player/Player.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Entity/Ball/Ball.h"
#include "Entity/Shit/Shit.h"

using namespace sf;

//--------------------------------------------------------------------------
void RugbyGoal::updateAABB()
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
		case GoalSide::Home:
			surface[0] = { cellPos.x - 9, cellPos.y };
			surface[1] = { cellPos.x + 10, cellPos.y + 2 };

			halfField[0] = { cellPos.x - 9, cellPos.y - 7};
			halfField[1] = { cellPos.x + 10, cellPos.y  };
			break;
	
		case GoalSide::Away:
			surface[0] = { cellPos.x - 9, cellPos.y - 2 };
			surface[1] = { cellPos.x + 10, cellPos.y };

			halfField[0] = { cellPos.x - 9, cellPos.y  };
			halfField[1] = { cellPos.x + 10, cellPos.y + 7};
			break;
	}

	m_blastZone = AABB(Vector2f((surface[0].x + 0.5f) * tile.x, (surface[0].y + 0.5f) * tile.y), sf::IntRect(0, 0, tile.x * (surface[1].x - surface[0].x), tile.y * (surface[1].y - surface[0].y)));
	m_halfField = AABB(Vector2f((halfField[0].x + 0.5f) * tile.x, (halfField[0].y + 0.5f) * tile.y), sf::IntRect(0, 0, tile.x * (halfField[1].x - halfField[0].x), tile.y * (halfField[1].y - halfField[0].y)));

	m_goalHeight = 40.0f;
	m_offsetX = 16.0f;
}

//--------------------------------------------------------------------------
void RugbyGoal::checkGoal(Entity * _entity, const sf::Vector2f & _goalPos, const TileInfo & _tile, GoalScoredType _type)
{
	Player * player = dynamic_cast<Player*>(_entity->getParent());

	// Check try
	if (_entity->getLastDropZoneAABB() != &this->m_blastZone)
	{
		if (player && _entity->isPicked() == false)
		{
			if (_entity->m_collisionAABB.intersects(m_blastZone) && player->m_collisionAABB.intersects(m_blastZone) && length(_entity->m_velocity) < 0.001f)
			{
				const auto & info = PlayerTypeInfo::get(player->getPlayerType());

				Ball * ball = dynamic_cast<Ball*>(_entity);
				Shit * shit = dynamic_cast<Shit*>(_entity);

				if ((ball && ball->isRugbyBall()) /*|| (shit && shit->getShitType() == ShitType::Rugby)*/)
				{
					player->addGoalScored(this, 5, 0.0f, GoalType::Rugby, m_goalSide);
					player->addScore(50);
					player->celebrate();
					_entity->setLastDropZoneAABB(&this->m_blastZone);
					return;
				}
			}
		}
	}
	else if (!_entity->m_collisionAABB.intersects(m_blastZone))
	{
		_entity->setLastDropZoneAABB(nullptr);
	}

	// Check drop
	const Vector2f & pos = _entity->getPosition();
	const Vector2f & prev = _entity->m_previousPos;

	const float height = _entity->m_height;

	if (_entity->m_firstFrame == false && !_entity->isPicked())
	{
		if (pos != prev)
		{
			const bool passed = GoalSide::Home == m_goalSide ? pos.y >= _goalPos.y && prev.y < _goalPos.y : pos.y < _goalPos.y && prev.y >= _goalPos.y;

			if (passed && height > m_goalHeight)
			{
				if (pos.x >= _goalPos.x + m_offsetX - _tile.m_box.width / 2 && pos.x <= _goalPos.x + m_offsetX + _tile.m_box.width / 2)
				{
					//debugPrint("GOAL 0x%08X!\n", this);
					Player * player = dynamic_cast<Player*>(_entity->getParent());
					if (player)
					{
						const float dist = length(pos - player->getPosition());

						const auto & info = PlayerTypeInfo::get(player->getPlayerType());
						if (_type == GoalScoredType::Ball)
						{
							player->addGoalScored(this, 3, dist, GoalType::Rugby, m_goalSide);
							player->addScore(dist / 5);
							player->celebrate();
						}
						else if (_type == GoalScoredType::Shit && (PlayerTypeInfo::Flags::ShitRugby & info.flags))
						{
							player->addGoalScored(this, 3, dist, GoalType::Rugby, m_goalSide);
							player->addScore(dist / 20);
							player->celebrate();
						}
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