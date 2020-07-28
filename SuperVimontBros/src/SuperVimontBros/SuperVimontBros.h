#pragma once

#include "Game/Game.h"
#include "Entity/Bonus/Bonus.h"
#include "Entity/Goal/Goal.h"

class Enemy;
class Ball;
class Shit;
class Vehicle;
class Goal;

enum class PlayerType : u8;

//--------------------------------------------------------------------------
enum class GameState
{
	Title = 0,
	Play,
	Win,
	Loose,

	Count
};

struct RadZone
{
	sf::Vector2f position;
	float radius;
	Player * owner;
	sf::Clock life;
};

//--------------------------------------------------------------------------
class SuperVimontBros : public Game
{
	public:
		static SuperVimontBros & get();

		bool init(bool _reinit = false) final override;
		void reset() final override;
		void deinit() final override;

		bool update() final override;
		bool draw() final override;

		void updateViewports() override;

		bool drawMode(sf::RenderTexture & _dst) override;
		void drawOverlay(sf::RenderTexture & _dst) override;

		bool isValidTilePos(const sf::Vector2u & _pos) const;
		TryMoveResult tryMove(Actor * _actor, sf::Vector2f & _moveVec) const override;
		bool isTileColliding(tileIndex _tile) const override;
		bool isTileObjectColliding(objectIndex _obj) const override;

		u32 getActivePlayerCount() const;

		void onPlayerAdd();
		void onGoalScored(Entity * _entity, uint _points, float _distance, GoalType _goalType, GoalSide _goalSide);

	protected:
		void createViewports();
		bool drawScore(Viewport & _viewport, sf::RenderTexture & _surface) final override;
		void killAllZombies();

	private:
		void populateLevel() override;
		void populateCell(SpawnList & _spawns, const CellInfo & _cell, uint _x, uint _y) override;

		void updateOverlay(float _dt);

		void updateBalls();

	public:
		using Super = Game;

		std::vector<Player*>	m_players; 
		ObjectList<Enemy*>		m_enemies;
		ObjectList<Vehicle*>	m_vehicles;
		ObjectList<Ball*>		m_balls;
		ObjectList<Shit*>		m_shits;
		ObjectList<Bonus*>		m_bonuses;
		ObjectList<Goal*>		m_goals;

		std::vector<Ball*>		m_footballs;
		std::vector<Ball*>		m_rugbyBalls;
		sf::Vector2u			m_score[(uint)GoalType::Count];
		
		struct GameStateData
		{
			std::vector<Entity*> m_entities;
		};

		GameState				m_gameState = GameState::Title;
		GameStateData			m_gameStateData[(u32)GameState::Count];

		sf::Texture				m_title;

		enum class TitleMenu
		{
			Play = 0,
			//Edit,
			//Credits,

			Count
		};
		TitleMenu				m_titleMenu = TitleMenu::Play;

		struct ControllerState
		{
			PlayerType playerType;
			bool enabled;
			Entity * icon;
			sf::Clock keyDelay;
		};
		std::vector<ControllerState>	m_selectedPlayerType;

		void					menuUp();
		void					menuDown();
		void					menuButton(unsigned int _button);

		uint					getMaxControllers();
		void					updateTitle(GameStateData & _data);
		void					drawTitle(sf::RenderTexture & _dst);
		uint					createPlayers();
		void					deleteGameStateData(GameState _gameState);

		void					drawBorders(sf::RenderTexture & _dst);

		void					addRadZone(const RadZone & _radZone);
		void					updateRadZones();

		std::vector<RadZone>	m_radZones;
		sf::Music				m_backgroundMusic;
};