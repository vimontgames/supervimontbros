#include "Precomp.h"
#include "SuperVimontBros.h"
#include "PostProcess/PostProcess.h"
#include "Viewport/Viewport.h"
#include "Controller/Controller.h"
#include "Message/MessageList.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Player/Player.h"
#include "Entity/Ball/Ball.h"
#include "Entity/Shit/Shit.h"
#include "Entity/Bullet/Bullet.h"
#include "Entity/Goal/Football/FootballGoal.h"
#include "Entity/Goal/Rugby/RugbyGoal.h"
#include "Entity/Bonus/Bonus.h"
#include "Entity/Vehicle/Peugeot3008/Peugeot3008.h"
#include "Entity/Vehicle/Clio/Clio.h"
#include "Entity/Dog/Dog.h"
#include "Entity/Flag/Flag.h"
#include "Entity/ElectricBox/ElectricBox.h"
#include "Entity/Breakable/Breakable.h"
#include "Tiles/LevelTile.h"
#include "Tiles/LevelObject.h"
#include "Tiles/SpriteModel.h"
#include "Tiles/LevelDecal.h"
#include "Menu/MenuTitle.h"

using namespace sf;

//--------------------------------------------------------------------------
SuperVimontBros & SuperVimontBros::get()
{
	return (SuperVimontBros&)Game::get();
}

//--------------------------------------------------------------------------
SuperVimontBros::SuperVimontBros() : m_menuTitle(new MenuTitle())
{
}

//--------------------------------------------------------------------------
SuperVimontBros::~SuperVimontBros()
{
	SAFE_DELETE(m_menuTitle);
}

//--------------------------------------------------------------------------
bool SuperVimontBros::init(bool _reinit)
{
	// Load sprite LUT
	m_spriteLut.loadFromFile("SuperVimontBros/data/img/lut/default.psd");
	m_spriteLut.setSmooth(true);
	m_spriteLutCount = m_spriteLut.getSize().y / 16;

	// Load sprite tiles
	m_spritesTile.init("SuperVimontBros/data/img/sprites.psd", { 32,32 }, { SPRITE_TILEMAP_WIDTH, SPRITE_TILEMAP_HEIGHT }, TilesType::SpriteTiles, _reinit);

	// Load level tiles
	m_levelTiles.init("SuperVimontBros/data/img/tiles.psd", { 32,32 }, { BACKGROUND_TILEMAP_WIDTH, BACKGROUND_TILEMAP_HEIGHT}, TilesType::LevelTiles, _reinit);

	// Load decals
	m_decalTiles.init("SuperVimontBros/data/img/decals.psd", { 32,32 }, { DECAL_TILEMAP_WIDTH, DECAL_TILEMAP_HEIGHT }, TilesType::DecalTiles, _reinit);

	// Load objects tiles
	m_objectTiles.init("SuperVimontBros/data/img/objects.psd", { 32,32 }, { OBJECT_TILEMAP_WIDTH, OBJECT_TILEMAP_HEIGHT }, TilesType::ObjectTiles, _reinit);
	
	// Init game
	Game::init(_reinit);

	// Load title
	m_menuTitle->init(_reinit); 

	if (!_reinit)
	{
		// Load level
		m_level.init("SuperVimontBros/data/level/000.lvl", { 196, 41 }, &m_levelTiles, &m_decalTiles, &m_objectTiles);

		// load music
		bool loaded = m_backgroundMusic.openFromFile("SuperVimontBros/data/music/JTRNET18_B_Maffi-Turtle-Riddim.ogg");
		m_backgroundMusic.play();
		m_backgroundMusic.setVolume(10);
		m_backgroundMusic.setLoop(true);
	}

	m_backgroundMusic.play();

	// Create level
	populateLevel();

	getDefaultViewport().reset();

	// Reset Players
	for (auto & info : m_selectedPlayerType)
	{
		info.enabled = false;
	}
	
	for (uint i = 0; i < (uint)GoalType::Count; ++i)
	{
		m_score[i] = { 0,0 };
	}

	// reset menus
	m_gameState = GameState::Title;
	m_menuTitle->setTitleMenu(MenuTitle::SubMenu::Main);

	m_postProcess->m_blur.set(0.5f);
	m_postProcess->m_color.set({ 0.1f, 0.1f, 0.1f });
	m_viewports[1]->setCameraOffset({ 1288, 200 + 6 * 32 });

	return true;
}

//--------------------------------------------------------------------------
bool SuperVimontBros::drawMode(sf::RenderTexture & _dst)
{
	bool result = Super::drawMode(_dst);

	if (m_debugDisplay)
	{
		if (m_gameState != GameState::Play && m_editorMode == EditorMode::None)
		{
			static const char * gameStateNames[] =
			{
				"Titre",
				"Jeu",
				"Victoire",
				"Défaites"
			};
			static_assert(sizeof(gameStateNames) / sizeof(gameStateNames[0]) == (uint)GameState::Count);

			m_debugString += String(gameStateNames[(u32)m_gameState]) + "\n";
		}

		return result;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------------------
uint SuperVimontBros::createPlayers()
{
	// Reset players
	uint numPlayers = 0;

	const Vector2f start[] = 
	{
		Vector2f(320, 6 * 32 + 144 + 64),
		Vector2f(320, 6 * 32 + 144),
		Vector2f(320, 6 * 32 + 144 + 32),
		Vector2f(320, 6 * 32 + 144 + 96)
	};

	u8 count[(uint)PlayerType::Count];
	for (uint c = 0; c < COUNT_OF(count); ++c)
	{
		count[c] = 0;
	}

	for (u32 c = 0; c < COUNT_OF(m_selectedPlayerType); ++c)
	{
		const auto & selected = m_selectedPlayerType[c];

		if (selected.enabled)
		{
			const auto playerIndex = count[(uint)selected.playerType];
			Player * player = new Player((ControllerID)c, selected.playerType, playerIndex);

			m_players.push_back(player);

			player->init();
			player->activate();
			player->playAnimation(Animation::Idle, player->m_faceLeft);
			player->setPosition(start[numPlayers]);
			player->setShaderID(m_visualShader);
			player->setLutIndex(PlayerTypeInfo::get(selected.playerType).palette[playerIndex]);

			// Actors must be added after Game::init
			registerVisual(player);
			SAFE_RELEASE(player);

			count[(uint)selected.playerType]++;
			numPlayers++;
		}
	}

	if (numPlayers)
	{
		onUpdateScale();
		createViewports();
	}

	return numPlayers;
}

//--------------------------------------------------------------------------
uint SuperVimontBros::getMaxControllers()
{
	const uint controllerCount = Controller::getControllerCount();
	const uint maxPlayers = min(controllerCount, (uint)4);
	return maxPlayers;
}

//--------------------------------------------------------------------------
void SuperVimontBros::drawBorders(RenderTexture & _dst)
{
	const auto playerCount = m_players.size();

	if (playerCount == 2 || playerCount == 4)
	{
		RectangleShape splitV;
					   splitV.setFillColor({ 0,0,0,0 });
					   splitV.setOutlineColor({ 0,0,0,255 });
					   splitV.setSize({ 1.0f, (float)m_screenSize.y });
					   splitV.setOutlineThickness(-1);
					   splitV.setPosition({ (float)m_screenSize.x / 2 - 1.0f, 0.0f });

		_dst.draw(splitV);
	}
	else if (playerCount == 3)
	{
		RectangleShape splitV;
					   splitV.setFillColor({ 0,0,0,0 });
					   splitV.setOutlineColor({ 0,0,0,255 });
					   splitV.setSize({ 1.0f, (float)m_screenSize.y / 2 });
					   splitV.setOutlineThickness(-1);
					   splitV.setPosition({ (float)m_screenSize.x / 2 - 1.0f, 0.0f });

		_dst.draw(splitV);
	}

	if (playerCount > 2)
	{
		RectangleShape splitH;
					   splitH.setFillColor({ 0,0,0,0 });
					   splitH.setOutlineColor({ 0,0,0,255 });
					   splitH.setSize({ (float)m_screenSize.x, 1.0f });
					   splitH.setOutlineThickness(-1);
					   splitH.setPosition({ 0.0f, (float)m_screenSize.y / 2 - 1.0f });

		_dst.draw(splitH);
	}
}

//--------------------------------------------------------------------------
void SuperVimontBros::drawOverlay(sf::RenderTexture & _dst)
{
	auto & surface = _dst;

	if (m_editorMode == EditorMode::None)
	{
		GameStateData & data = m_gameStateData[(uint)m_gameState];

		for (Entity * entity : data.m_entities)
		{
			entity->drawShadow(surface);
			entity->draw(surface);
		}

		switch (m_gameState)
		{
			case GameState::Title:
				m_menuTitle->draw(_dst);
				break;

			case GameState::Play:
				drawBorders(_dst);
				break;
		}

	}

	Super::drawOverlay(_dst);
}

//--------------------------------------------------------------------------
u32 SuperVimontBros::getActivePlayerCount() const
{
	u32 activePlayers = 0;
	for (Player * player : m_players)
	{
		if (player->isActive())
		{
			activePlayers++;
		}
	}
	return activePlayers;
}

//--------------------------------------------------------------------------
void SuperVimontBros::createViewports()
{
	uint activePlayers = getActivePlayerCount();

	const float gameScale = m_scale;

	const bool split = m_splitScreen && activePlayers > 1;

	// Keep only defaults viewports
	for (uint i = 2; i < m_viewports.size(); ++i)
	{
		SAFE_DELETE(m_viewports[i]);
	}
	m_viewports.resize(min((size_t)2, m_viewports.size()));

	const u32 w = m_viewports[0]->getView().getSize().x * m_scale / gameScale;
	const u32 h = m_viewports[0]->getView().getSize().y * m_scale / gameScale;
	Vector2u backbufferSize = { w, h };

	if (split)
	{
		if (activePlayers == 2)
		{
			// Resize Viewport 1
			m_viewports[1]->init(backbufferSize.x / 2, backbufferSize.y, 0, 0);

			// Create Viewports 2
			if (m_viewports.size() <= 2)
				m_viewports.push_back(new Viewport("Game #2"));

			m_viewports[2]->init(backbufferSize.x / 2, backbufferSize.y, backbufferSize.x / 2, 0);
		}
		else if (activePlayers == 3)
		{
			// Resize Viewport 1
			m_viewports[1]->init(backbufferSize.x / 2, backbufferSize.y / 2, 0, 0);

			// Create Viewports 2
			if (m_viewports.size() <= 2)
				m_viewports.push_back(new Viewport("Game #2"));

			m_viewports[2]->init(backbufferSize.x / 2, backbufferSize.y / 2, backbufferSize.x / 2, 0);

			// Create Viewports 3
			if (m_viewports.size() <= 3)
				m_viewports.push_back(new Viewport("Game #3"));

			m_viewports[3]->init(backbufferSize.x, backbufferSize.y / 2, 0, backbufferSize.y / 2);
		}
		else if (activePlayers == 4)
		{
			// Resize Viewport 1
			m_viewports[1]->init(backbufferSize.x / 2, backbufferSize.y / 2, 0, 0);

			// Create Viewports 2
			if (m_viewports.size() <= 2)
				m_viewports.push_back(new Viewport("Game #2"));

			m_viewports[2]->init(backbufferSize.x / 2, backbufferSize.y / 2, backbufferSize.x / 2, 0);

			// Create Viewports 3
			if (m_viewports.size() <= 3)
				m_viewports.push_back(new Viewport("Game #3"));

			m_viewports[3]->init(backbufferSize.x / 2, backbufferSize.y / 2, 0, backbufferSize.y / 2);

			// Create Viewports 4
			if (m_viewports.size() <= 4)
				m_viewports.push_back(new Viewport("Game #4"));

			m_viewports[4]->init(backbufferSize.x / 2, backbufferSize.y / 2, backbufferSize.x / 2, backbufferSize.y / 2);
		}
	}
	else
	{
		// Restore Viewport 1
		m_viewports[1]->init(backbufferSize.x, backbufferSize.y, 0, 0);
	}

	// Assign viewports to players
	uint viewportIndex = 0;

	for (u32 i = 0; i < m_players.size(); ++i)
	{
		Player * player = m_players[i];

		if (player->isActive())
		{
			if (split)
			{
				auto & viewport = *m_viewports[1 + viewportIndex];
				player->setViewport(viewport);
				viewport.setCameraOffset(player->getPosition());
				viewportIndex++;
			}
			else
			{
				auto & viewport = *m_viewports[1];
				player->setViewport(viewport);
				viewport.setCameraOffset(player->getPosition());
			}
		}
	}
}

//--------------------------------------------------------------------------
void SuperVimontBros::onGoalScored(Entity * _entity, uint _points, float _distance, GoalType _goalType, GoalSide _goalSide)
{
	auto & score = m_score[(uint)_goalType];

	if (GoalSide::Home == _goalSide)
	{
		score.y += _points;	// increase score for the AWAY team
	}
	else
	{
		score.x += _points;	// increase score for the HOME team
	}

	int F = 30;
	int f = 15;
	int title_fontSize = 2;

	const Player * player = dynamic_cast<Player*>(_entity);

	const char * zombieName = "ZOMBIE";
	if (!player)
	{
		if (Enemy * enemy = dynamic_cast<Enemy*>(_entity))
		{
			if (enemy->getSide() == GoalSide::Home)
			{
				static const char * names[] =
				{
					"JEAN NEYMAR",
					"CAVAZOMBI",
					"M'BAPPROUT"
				};

				zombieName = names[enemy->getGUID() % COUNT_OF(names)];
			}
			else if (enemy->getSide() == GoalSide::Away)
			{
				static const char * names[] =
				{
					"ZOMBIMAN",
					"SKELETIC"
				};

				zombieName = names[enemy->getGUID() % COUNT_OF(names)];
			}
		}
	}

	for (auto * viewport : m_viewports)
	{
		if (viewport->isEnabled() == false)
			continue;

		if (viewport->isVisible(_entity) == false)
			continue;

		auto & msgList = viewport->getMessageList();
		auto viewportSize = viewport->getSize();

		const sf::Color color = { 255,255,255,255 };
		const auto duration1 = 3000.0f;
		const auto duration2 = 2500.0f;

		float distanceInMeters = _distance * 4.0f / 32.0f;

		switch (_goalType)
		{
			case GoalType::Football:
				msgList.print(duration1, F, color, viewportSize.x / 2 + title_fontSize, viewportSize.y / 2 - F + title_fontSize, "PSG %u-%u OM", score.x, score.y);
				msgList.print(duration2, f, color, viewportSize.x / 2 + title_fontSize, viewportSize.y / 2 + f / 2 + title_fontSize, "BUT DE %s ", player ? _entity->name().toAnsiString().c_str() : zombieName);
				msgList.print(duration2, f, color, viewportSize.x / 2 + title_fontSize, viewportSize.y / 2 + 4 * f / 2 + title_fontSize, "%.2f METRES", distanceInMeters);
				break;

			case GoalType::Rugby:
				if (_points == 5)
				{
					msgList.print(duration1, F, color, viewportSize.x / 2 + title_fontSize, viewportSize.y / 2 - F + title_fontSize, "%u-%u", score.x, score.y);
					msgList.print(duration2, f, color, viewportSize.x / 2 + title_fontSize, viewportSize.y / 2 + f / 2 + title_fontSize, "ESSAI DE %s ", player ? _entity->name().toAnsiString().c_str() : zombieName);
				}
				else if (_points == 3)
				{
					msgList.print(duration1, F, color, viewportSize.x / 2 + title_fontSize, viewportSize.y / 2 - F + title_fontSize, "%u-%u", score.x, score.y);
					msgList.print(duration2, f, color, viewportSize.x / 2 + title_fontSize, viewportSize.y / 2 + f / 2 + title_fontSize, "DROP DE %s ", player ? _entity->name().toAnsiString().c_str() : zombieName);
					msgList.print(duration2, f, color, viewportSize.x / 2 + title_fontSize, viewportSize.y / 2 + 4 * f / 2 + title_fontSize, "%.2f METRES", distanceInMeters);
				}
				break;
		}
	}
}

//--------------------------------------------------------------------------
void SuperVimontBros::reset()
{
	for (Viewport * viewport : m_viewports)
	{
		viewport->getMessageList().clear();
	}

	for (u32 i = 0; i < m_players.size(); ++i)
	{
		unregisterVisual(m_players[i]);
	}
	m_players.clear();

	m_enemies.clear();
	m_balls.clear();
	m_footballs.clear();
	m_rugbyBalls.clear();
	m_goals.clear();
	m_vehicles.clear();
	m_shits.clear();
	m_bullets.clear();
	m_bonuses.clear();
	m_radZones.clear();

	Game::reset();
}

//--------------------------------------------------------------------------
void SuperVimontBros::deinit()
{
	// Game::deinit will take care of calling (virtual) SuperVimontBros::reset
	for (uint i = 0; i < (uint)GameState::Count; ++i)
	{
		deleteGameStateData((GameState)i);
	}

	Game::deinit();
}

//--------------------------------------------------------------------------
void SuperVimontBros::populateCell(SpawnList & _spawns, const CellInfo & _cell, uint _x, uint _y)
{
	const auto levelTile = (LevelTile)_cell.tile.index;
	switch (levelTile)
	{
		case LevelTile::ZombieHole:
			_spawns.push_back({ new Enemy(EnemyType::Zombie), { -2,8 } });
			break;
	}

	const auto levelObject = (LevelObject)_cell.obj.index;
	switch (levelObject)
	{
		// Ennemies
		case LevelObject::ZombiePatrick:
			_spawns.push_back({ new Enemy(EnemyType::ZombiePatrick), { -2,8 } });
			break;

		case LevelObject::Zombie:
			_spawns.push_back({ new Enemy(EnemyType::Zombie), { -2,8 } });
			break;

		case LevelObject::ZombiePlaya:
			_spawns.push_back({ new Enemy(EnemyType::ZombiePlaya), { -2,8 } });
			break;

		case LevelObject::ZombieCowboy:
			_spawns.push_back({ new Enemy(EnemyType::ZombieCowboy), { -2,8 } });
			break;

		case LevelObject::YellowVest:
			_spawns.push_back({ new Enemy(EnemyType::YellowVest), { -2,8 } });
			break;

		case LevelObject::Footballer:
			_spawns.push_back({ new Enemy(EnemyType::Footballer), { -2,8 } });
			break;

		case LevelObject::Footballer2:
			_spawns.push_back({ new Enemy(EnemyType::Footballer2), { -2,8 } });
			break;

		case LevelObject::Goalkeeper:
			_spawns.push_back({ new Enemy(EnemyType::Goalkeeper)});
			break;

		case LevelObject::Goalkeeper2:
			_spawns.push_back({ new Enemy(EnemyType::Goalkeeper2) });
			break;

		case LevelObject::Rugbyman:
			_spawns.push_back({ new Enemy(EnemyType::Rugbyman), { -2,8 } });
			break;

		// Flags
		case LevelObject::Flag:
			_spawns.push_back({ new Flag(FlagType::YellowRed) });
			break;

		case LevelObject::FrenchFlag:
			_spawns.push_back({ new Flag(FlagType::France) });
			break;
							
		// Balls
		case LevelObject::FootballWhite:
			_spawns.push_back({ new Ball(BallType::FootballWhite) });
			break;

		case LevelObject::FootballYellow:
			_spawns.push_back({ new Ball(BallType::FootballYellow) });
			break;

		case LevelObject::BeachBall			:
			_spawns.push_back({ new Ball(BallType::BeachBall) });
			break;

		case LevelObject::RugbyBall:
			_spawns.push_back({ new Ball(BallType::Rugby) });
			break;

		case LevelObject::RugbyBallWhite:
			_spawns.push_back({ new Ball(BallType::RugbyWhite) });
			break;

		case LevelObject::TennisBall:
			_spawns.push_back({ new Ball(BallType::Tennis) });
			break;

		// Dog NPCs
		case LevelObject::DogJune:
			_spawns.push_back({ new Dog(DogType::June) });
			break;

		case LevelObject::Praline:
			_spawns.push_back({ new Dog(DogType::Praline) });
			break;

		case LevelObject::DogLucky:
			_spawns.push_back({ new Dog(DogType::Lucky) });
			break;

		// Bonuses
		case LevelObject::Pizza:
			_spawns.push_back({ new Bonus("Pizza", BonusType::Pizza) });
			break;

		case LevelObject::Apple:
			_spawns.push_back({ new Bonus("Apple", BonusType::Apple) });
			break;

		case LevelObject::Eclair:
			_spawns.push_back({ new Bonus("ChocolateEclair", BonusType::ChocolateEclair)});
			break;

		case LevelObject::Crunch:
			_spawns.push_back({ new Bonus("Crunch", BonusType::Crunch) });
			break;

		case LevelObject::Wine:
			_spawns.push_back({ new Bonus("Wine", BonusType::Wine) });
			break;

		case LevelObject::IceCream:
			_spawns.push_back({ new Bonus("IceCream", BonusType::IceCream) });
			break;

		case LevelObject::Coca:
			_spawns.push_back({ new Bonus("Coca", BonusType::Coca) });
			break;

		case LevelObject::Water:
			_spawns.push_back({ new Bonus("Water", BonusType::Water) });
			break;

		case LevelObject::Hotdog:
			_spawns.push_back({ new Bonus("Hotdog", BonusType::Hotdog) });
			break;

		case LevelObject::BluePill:
			_spawns.push_back({ new Bonus("BluePill", BonusType::BluePill) });
			break;

		case LevelObject::RedPill:
			_spawns.push_back({ new Bonus("RedPill", BonusType::RedPill) });
			break;

		case LevelObject::GreenPill:
			_spawns.push_back({ new Bonus("GreenPill", BonusType::GreenPill) });
			break;

		case LevelObject::Bone:
			_spawns.push_back({ new Bonus("Bone", BonusType::Bone) });
			break;

		case LevelObject::Chicken:
			_spawns.push_back({ new Bonus("Chicken", BonusType::Chicken ) });
			break;

		case LevelObject::PureeSausage:
			_spawns.push_back({ new Bonus("PureeSausage", BonusType::PureeSausage) });
			break;

		// Soccer
		case LevelObject::CelebrateGoal:
			_spawns.push_back({ new FootballGoal() });
			break;

		// Rugby
		case LevelObject::RugbyGoal:
			_spawns.push_back({ new RugbyGoal() });
			break;

		// Cars
		case LevelObject::Peugeot3008:
			_spawns.push_back({ new Peugeot3008() });
			break;

		case LevelObject::Peugeot206:
			_spawns.push_back({ new Clio(ClioType::Red) });
			break;

		case LevelObject::RenaultClio:
			_spawns.push_back({ new Clio(ClioType::Blue) });
			break;

		case LevelObject::SandCastle:
			_spawns.push_back({ new Breakable(BreakableType::SandCastle) });
			break;

		case LevelObject::ElectricBox:
			_spawns.push_back({ new ElectricBox((rand() & 1) ? true : false) });
			break;
	}

	Super::populateCell(_spawns, _cell, _x, _y);
}

//--------------------------------------------------------------------------
void SuperVimontBros::populateLevel()
{
	Super::populateLevel();
}

//--------------------------------------------------------------------------
void SuperVimontBros::killAllZombies()
{
	for (uint i = 0; i < m_enemies.size(); ++i)
	{
		auto * enemy = m_enemies[i];
		enemy->kill(nullptr);
	}
}

//--------------------------------------------------------------------------
void SuperVimontBros::updateBalls()
{
	m_footballs.clear();
	m_rugbyBalls.clear();

	for (auto * ball : m_balls.get())
	{
		const bool foot = ball->isFootBall();
		const bool rugby = ball->isRugbyBall();
		if (foot || rugby)
		{
			const auto flags = Goal::isInsideField(ball);

			if (foot && asBool(flags & TerrainTestFlags::IsInFootballField))
				m_footballs.push_back(ball);
			else if (rugby && asBool(flags & TerrainTestFlags::IsInRugbyField))
				m_rugbyBalls.push_back(ball);
		}
	}
}

//--------------------------------------------------------------------------
bool SuperVimontBros::update()
{
	Game::update();

	if (Keyboard::isKeyPressed(Keyboard::Key::F7))
	{
		if (m_enemies.size() > 0)
		{
			killAllZombies();
		}
		setKeyPressed();
	}

	if (Keyboard::isKeyPressed(Keyboard::Key::F8))
	{
		updateShaders();
		setKeyPressed();
	}

	updateRadZones();
	updateBalls();

	for (Viewport * viewport : m_viewports)
	{
		if (!viewport->isEnabled())
			continue;

		auto & messageList = viewport->getMessageList();

		if (m_editorMode == EditorMode::None)
		{
			sf::Vector2i m = { 0,0 };
			int activePlayers = 0;

			sf::Vector2i avg = { 0,0 };

			if (m_players.size() > 0)
			{
				for (u32 i = 0; i < m_players.size(); ++i)
				{
					const Player * player = m_players[i];

					if ((player->isActive() || m_viewports.size() > 1) && player->getViewport() == viewport)
					{
						activePlayers++;

						m.x = max(m.x, (int)m_players[i]->getPosition().x);
						m.y = max(m.y, (int)m_players[i]->getPosition().y);

						avg.x += (int)m_players[i]->getPosition().x;
						avg.y += (int)m_players[i]->getPosition().y;
					}
				}

				if (activePlayers > 0)
				{
					avg.x /= activePlayers;
					avg.y /= activePlayers;
				}
			}

			if (activePlayers > 0)
			{
				#if 0
				viewport->setCameraOffset({ (float)avg.x, (float)avg.y });
				#else
				viewport->setCameraOffset({ (float)m.x, (float)m.y });
				#endif
			}
		}
		else if (m_editorMode == EditorMode::PaintTile || m_editorMode == EditorMode::PaintObject || m_editorMode == EditorMode::PaintDecal)
		{
			getDefaultViewport().setCameraOffset(m_level.m_paintCamera);
		}

		auto & cameraOffset = viewport->getCameraOffset();

		const auto & viewportSize = viewport->getSize();

		cameraOffset.x = (float)max((int)cameraOffset.x, (int)(viewportSize.x / 2));
		cameraOffset.x = (float)min((int)cameraOffset.x, (int)(m_level.count().x * m_levelTiles.m_size.x - viewportSize.x / 2));

		cameraOffset.y = (float)max((int)cameraOffset.y, (int)(viewportSize.y / 2));
		cameraOffset.y = (float)min((int)cameraOffset.y, (int)(m_level.count().y * m_levelTiles.m_size.y - viewportSize.y / 2));

		if (m_editorMode == EditorMode::None || m_editorMode == EditorMode::PaintTile || m_editorMode == EditorMode::PaintObject || m_editorMode == EditorMode::PaintDecal)
		{
			viewport->getView().setSize((float)viewportSize.x, (float)viewportSize.y);
			viewport->getView().setCenter(cameraOffset);
		}
	}

	updateOverlay(m_dt);

	// Game::updateMessages() ?
	for (Viewport * viewport : m_viewports)
	{
		auto & messageList = viewport->getMessageList();
		messageList.update();

		auto * postProcess = viewport->getPostProcess();
		postProcess->update(m_dt);
	}

	m_messageList->update(); 
	m_postProcess->update(m_dt);

	return true;
}

//--------------------------------------------------------------------------
bool sortScores(const Player * _A, const Player * _B)
{
	return (_A->getScore()) > (_B->getScore()) || (_A->isActive() && !_B->isActive());
}

//--------------------------------------------------------------------------
void SuperVimontBros::updateOverlay(float _dt)
{
	if (m_editorMode == EditorMode::None)
	{

		bool anyPlayersAlive = false;
		bool anyPlayerActive = false;

		for (Player * player : m_players)
		{
			if (player->isActive())
			{
				if (player->m_hp != 0)
				{
					anyPlayersAlive = true;
				}

				anyPlayerActive = true;
			}
		}

		if (GameState::Play == m_gameState)
		{
			if (anyPlayerActive && m_enemies.size() == 0)
			{
				const float t = 1.0f;

				m_postProcess->m_blur.set(1.0f, t);
				m_postProcess->m_color.set({ 0.25f, 0.25f, 0.25f }, t);

				m_gameState = GameState::Win;
			}
			else if (anyPlayerActive && !anyPlayersAlive)
			{
				const float t = 1.0f;

				m_postProcess->m_saturation.set(0.0f, t);
				m_postProcess->m_blur.set(1.0f, t);
				m_postProcess->m_color.set({ 0.25f, 0.25f, 0.25f }, t);

				m_gameState = GameState::Loose;
			}
		}

		auto * messages = m_messageList;

		GameStateData & data = m_gameStateData[(u32)m_gameState];
		auto center = Vector2f(m_screenSize.x / 2, m_screenSize.y / 2);

		switch (m_gameState)
		{
		case GameState::Title:
			m_menuTitle->update(_dt, data);
			break;

		case GameState::Win:
		{
			if (data.m_entities.size() == 0)
			{
				Entity * podium = new Entity("Podium", m_spritesTile);
				podium->setImage({ 1,15 });
				podium->setPosition(center);

				data.m_entities.push_back(podium);

				uint index = 0;

				const Vector2f pos[3] =
				{
					center + Vector2f(0.0f, -21.0f),
					center + Vector2f(-20.0f, -15.0f),
					center + Vector2f(+20.0f, -13.0f),
				};

				std::vector<Player*> sorted = m_players;
				std::sort(sorted.begin(), sorted.end(), sortScores);

				for (Player * player : sorted)
				{
					if (index > 2)
						continue;

					Entity * entity = new Entity(player->name(), *player->m_tileSet);
					entity->m_animSequences = player->m_animSequences;
					entity->setPosition(pos[index]);
					entity->setColor(player->m_sprite.getColor());
					entity->setShaderID(player->m_shaderID);
					entity->setLutIndex(player->m_lutIndex);

					Animation anim[] =
					{
						Animation::CelebrateGoal,
						Animation::Idle,
						Animation::Idle
					};
					entity->playAnimation(anim[index], index == 2 ? true : false);

					index++;

					data.m_entities.push_back(entity);
				}

				int F = 32;
				int f = 16;
				int title_fontSize = 0;

				messages->print(-1.0f, F, { 255,255, 255, 255 }, m_screenSize.x / 2 + title_fontSize, m_screenSize.y * 1 / 4 - F / 2 + title_fontSize, "BRAVO");
				messages->print(-1.0f, f, { 255,255, 255, 255 }, m_screenSize.x / 2 + title_fontSize, m_screenSize.y * 1 / 4 + f + title_fontSize, "VOUS AVEZ GAGNÉ");
			}
		}
		break;

		case GameState::Loose:
		{
			if (data.m_entities.size() == 0)
			{
				uint index = 0;

				const Vector2f pos[3] =
				{
					center + Vector2f(0.0f, 0.0f),
					center + Vector2f(-20.0f, 0.0f),
					center + Vector2f(+20.0f, 0.0f),
				};

				std::vector<Player*> sorted = m_players;
				std::sort(sorted.begin(), sorted.end(), sortScores);

				for (Player * player : sorted)
				{
					Entity * entity = new Entity(player->name(), *player->m_tileSet);
					entity->m_animSequences = player->m_animSequences;
					entity->setPosition(pos[index]);
					entity->setColor(player->m_sprite.getColor());
					entity->setShaderID(player->m_shaderID);
					entity->setLutIndex(player->m_lutIndex);

					Animation anim[] =
					{
						Animation::Die,
						Animation::Die,
						Animation::Die
					};
					entity->playAnimation(anim[index], index == 2 ? true : false);

					index++;

					data.m_entities.push_back(entity);
				}

				int F = 32;
				int f = 16;
				int title_fontSize = 0;

				messages->print(-1.0f, F, { 255, 0, 0, 255 }, m_screenSize.x / 2 + title_fontSize, m_screenSize.y * 1 / 4 - F / 2 + title_fontSize, "PERDU");
				messages->print(-1.0f, f, { 255, 0, 0, 255 }, m_screenSize.x / 2 + title_fontSize, m_screenSize.y * 1 / 4 + f / 2 + title_fontSize, "PLUS AUCUN COEUR");
			}
		}
		break;
		}

		for (uint i = 0; i < (u32)GameState::Count; ++i)
		{
			GameStateData & data = m_gameStateData[i];
			const auto state = (GameState)i;
			if (state == m_gameState)
			{
				for (Entity * entity : data.m_entities)
				{
					entity->update(_dt);
				}
			}
			else
			{
				deleteGameStateData(state);
			}
		}
	}
}

//--------------------------------------------------------------------------
void SuperVimontBros::deleteGameStateData(GameState _gameState)
{
	GameStateData & data = m_gameStateData[(uint)_gameState];

	for (Entity * entity : data.m_entities)
	{
		SAFE_DELETE(entity);
	}
	data.m_entities.clear();
}

//--------------------------------------------------------------------------
bool SuperVimontBros::draw()
{
	Game::draw();

	return true;
}

//--------------------------------------------------------------------------
void SuperVimontBros::onPlayerAdd()
{
	const u32 activePlayerCount = getActivePlayerCount();
	
	//float newScale = m_scale;
	//if (activePlayerCount <= 1)
	//{
	//	newScale = 4;
	//}
	//else if (activePlayerCount == 2)
	//{
	//	newScale = 3;
	//}
	//else
	//{
	//	newScale = 2;
	//}
	//
	//if (newScale != m_scale)
	//{
	//	m_scale = newScale;
	//	onUpdateScale();
	//}

	updateViewports();
}

//--------------------------------------------------------------------------
void SuperVimontBros::updateViewports()
{
	createViewports();
}

//--------------------------------------------------------------------------
bool SuperVimontBros::drawScore(Viewport & _viewport, sf::RenderTexture & _surface)
{
	uint index = 0;

	for (uint p = 0; p < m_players.size(); ++p)
	{
		const Player * player = m_players[p];

		if (&_viewport != player->getViewport())
			continue;

		sf::Color color;
		if (player->isActive())
		{
			color = { 255,255,255,255 };
		}
		else
		{
			color = { 255,255,255,128 };
		}

		const uint playerOffset = index * m_screenSize.x / m_players.size();
		const uint textOffset = 32;

		char temp[512];
		sprintf_s(temp, "%.5s%c\n%.5u  %.3u  %.3u  %.3u\n", 
			player->name().toAnsiString().c_str(), player->getPlayerTypeSubIndex() != 0 ? player->getPlayerTypeSubIndex() + '0' + 1 : ' ', 
			player->getScore(), player->getZombieKilled(), 
			player->getGoalScored(GoalType::Football),
			player->getGoalScored(GoalType::Rugby)
		);

		if (m_debugDisplay)
		{
			sprintf_s(temp, "%sx:%.0f y:%.0f", temp, player->getPosition().x, player->getPosition().y);

		}

		Text debugText;
			 debugText.setString(temp);
			 debugText.setFont(getFont(GameFont::Dlx8));
			 debugText.setCharacterSize(8);
			 debugText.setPosition((float)(playerOffset + textOffset), 0);
			 debugText.setFillColor(color);

		const uint hp = player->m_hp;
		const uint offset = 5 * 8;
		for (uint h = 0; h < Entity::s_maxHp / 4; ++h)
		{
			const uint heartOffset = offset + h * 8 + 9;

			uint img = 4;

			if (hp / 4 > h)
			{
				img = 0;
			}
			else if( (hp+1) / 4 > h)
			{
				img = 1;
			}
			else if ((hp + 2) / 4 > h)
			{
				img = 2;
			}
			else if ((hp + 3) / 4 > h)
			{
				img = 3;
			}

			Vector2u coords = { img, 14 };

			TileInfo & info = m_spritesTile.m_tileInfos.get(coords);

			Sprite heart;
				   heart.setTexture(m_spritesTile.m_texture);
				   heart.setTextureRect(m_spritesTile.getTileRect(coords));
				   heart.setPosition(Vector2f(playerOffset + textOffset + heartOffset + 4 + info.m_box.left + info.m_origin.x, 4 + info.m_box.top + +info.m_origin.y));
				   heart.setOrigin((Vector2f)info.m_origin);
				   heart.setColor(color);

			_surface.draw(heart);
		}

		// Zombies killed
		{
			Vector2u coords = { 5, 15 };
			TileInfo & info = m_spritesTile.m_tileInfos.get(coords);

			Sprite icon;
			icon.setTexture(m_spritesTile.m_texture);
			icon.setTextureRect(m_spritesTile.getTileRect(coords));
			icon.setPosition(Vector2f(playerOffset + textOffset + offset + 4 + info.m_box.left + info.m_origin.x + 8 - 1, 4 + info.m_box.top + info.m_origin.y + 8));
			icon.setOrigin((Vector2f)info.m_origin);
			icon.setColor(color);

			_surface.draw(icon);
		}

		// Football goals scored
		{
			Vector2u coords = { 6, 15 };
			TileInfo & info = m_spritesTile.m_tileInfos.get(coords);

			Sprite icon;
			icon.setTexture(m_spritesTile.m_texture);
			icon.setTextureRect(m_spritesTile.getTileRect(coords));
			icon.setPosition(Vector2f(playerOffset + textOffset + offset + 4 + info.m_box.left + info.m_origin.x + 8 + 1 * 40 - 1, 4 + info.m_box.top + info.m_origin.y + 8));
			icon.setOrigin((Vector2f)info.m_origin);
			icon.setColor(color);

			_surface.draw(icon);
		}

		// Rugby goals scored
		{
			Vector2u coords = { 7, 15 };
			TileInfo & info = m_spritesTile.m_tileInfos.get(coords);

			Sprite icon;
				   icon.setTexture(m_spritesTile.m_texture);
				   icon.setTextureRect(m_spritesTile.getTileRect(coords));
				   icon.setPosition(Vector2f(playerOffset + textOffset + offset + 4 + info.m_box.left + info.m_origin.x + 8 + 2 * 40 -1, 4 + info.m_box.top + info.m_origin.y + 8));
				   icon.setOrigin((Vector2f)info.m_origin);
				   icon.setColor(color);

			_surface.draw(icon);
		}

		_surface.draw(debugText);

		++index;
	}

	//if (m_debugDisplay)
	{
		for (auto & zone : m_radZones)
		{
			sf::CircleShape radius;
							radius.setPosition(zone.position - _viewport.getMinScreenCorner() - Vector2f(zone.radius, zone.radius));
							radius.setRadius(zone.radius);
							radius.setFillColor({ 0,255,0, 64});

			_surface.draw(radius);
		}
	}

	return true;
}

//--------------------------------------------------------------------------
bool SuperVimontBros::isTileColliding(tileIndex _tile) const
{
	const auto & info = m_levelTiles.m_tileInfos.get(m_levelTiles.indexToCoords(_tile));
	return asBool(TileFlags::Collide & info.m_flags);
}

//--------------------------------------------------------------------------
bool SuperVimontBros::isTileObjectColliding(objectIndex _obj) const
{
	const auto & info = m_objectTiles.m_tileInfos.get(m_objectTiles.indexToCoords(_obj));
	return asBool(TileFlags::Collide & info.m_flags);
}

//--------------------------------------------------------------------------
bool SuperVimontBros::isValidTilePos(const sf::Vector2u & _pos) const
{
	return _pos.x >= 0 && _pos.x < m_level.m_cells.size().x && _pos.y >= 0 && _pos.y < m_level.m_cells.size().y;
}

//--------------------------------------------------------------------------
bool overlap(float _p0, float _h0, float _p1, float _h1)
{
	if ((_p0 + _h0 < _p1 || _p0 + _h0 > _p1 + _h1) && (_p1 + _h1 < _p0 || _p1 + _h1 > _p0 + _h0))
		return false;

	return true;
}

//--------------------------------------------------------------------------
TryMoveResult SuperVimontBros::tryMove(Actor * _actor, sf::Vector2f & _moveVec) const
{
	TryMoveResult result = { true, true };

	const sf::Vector2f & pos = _actor->getPosition();
	const AABB & aabb = _actor->m_collisionAABB;

	const int k = _actor->m_collisionKernel;
	const int n = k-1;
	const float bounce = _actor->m_bounce;

	// Check Actors
	if (_actor->m_isCollidingWithOtherActors)
	{
		const uint actorCount = m_actors.size();
		for (uint i = 0; i < actorCount; ++i)
		{
			Actor * actor = m_actors[i];

			const bool isInactive = dynamic_cast<Player*>(actor) && (!dynamic_cast<Player*>(actor)->isActive() || dynamic_cast<Player*>(actor)->isInVehicle());

			if (actor == _actor || !actor->m_isColliderForOtherActors || isInactive /*|| _actor->m_parent == actor || actor->m_parent == _actor*/)
				continue;

			const bool overlapZ = actor->isImageSet() && overlap(_actor->m_height, _actor->getTileInfo().m_collisionZ, actor->m_height, actor->getTileInfo().m_collisionZ);

			if (overlapZ)
			{
				const AABB hBox = AABB(aabb, Vector2f(_moveVec.x + sign(_moveVec.x), 0));
				const bool collideX = hBox.intersects(actor->m_collisionAABB);

				const AABB vBox = AABB(aabb, Vector2f(0, _moveVec.y + sign(_moveVec.y)));
				const bool collideY = vBox.intersects(actor->m_collisionAABB);

				if (collideX || collideY)
				{
					_actor->onActorCollision(actor, _moveVec, collideX, collideY);
					actor->onBeingCollided(_actor, _moveVec, collideX, collideY);

					result.canMoveX |= collideX;
					result.canMoveY |= collideY;
				}
			}
		}
	}

	const Vector2i cellCount = (Vector2i)m_level.m_cells.size();
	const Vector2i tileSize = (Vector2i)m_levelTiles.m_size;

	const float leftBorder = 4;
	const float rightBorder = 4;
	const float topBorder = 4;
	const float bottomBorder = 4;

	// Check borders
	{
		const AABB hBox = AABB(aabb, Vector2f(_moveVec.x + sign(_moveVec.x), 0));
		const AABB left({ -1.0f * tileSize.x + leftBorder, -1.0f * tileSize.y }, { 0,0,tileSize.x,tileSize.y*(cellCount.y + 1) });
		const AABB right({ (float)(cellCount.x * tileSize.x) - rightBorder, -1.0f * tileSize.y }, { 0,0,tileSize.x,tileSize.y*(cellCount.y + 1) });

		if (hBox.intersects(left) || hBox.intersects(right))
		{
			_actor->onTileCollision(-1, _moveVec, true, false);
			result.canMoveX = false;
		}
		
		const AABB vBox = AABB(aabb, Vector2f(0, _moveVec.y + sign(_moveVec.y)));
		const AABB top({ -1.0f * tileSize.x, -1.0f * tileSize.y + topBorder }, { 0,0,tileSize.x*(cellCount.x + 1),tileSize.y });
		const AABB bottom({ -1.0f * tileSize.x, (float)(cellCount.y * tileSize.y) - bottomBorder }, { 0,0,tileSize.x*(cellCount.x + 1),tileSize.y });
		if (vBox.intersects(top) || vBox.intersects(bottom))
		{
			_actor->onTileCollision(-1, _moveVec, false, true);
			result.canMoveY = false;
		}
	}

	// Check Tiles
	if (_actor->m_isCollidingWithTiles)
	{
		for (int y = -k/2; y < (k+1)/2; ++y)
		{
			for (uint x = 0; x < n; ++x)
			{
				const Vector2u cellPosX = m_level.getTilePos(pos + Vector2f(sign(_moveVec.x) * m_levelTiles.m_size.x * x, y * (int)m_levelTiles.m_size.y));
				if (isValidTilePos(cellPosX))
				{
					const CellInfo & cellX = m_level.get(cellPosX);
					const TileInfo & objectTileInfoX = m_levelTiles.m_tileInfos.get(m_levelTiles.indexToCoords(cellX.tile.index));

					if (isTileColliding(cellX.tile.index))
					{
						// ... vs Tile AABB
						Vector2f bPos = Vector2f(cellPosX.x * m_levelTiles.m_size.x + objectTileInfoX.m_origin.x, cellPosX.y * m_levelTiles.m_size.y + objectTileInfoX.m_origin.y);
						const AABB bBox(bPos, objectTileInfoX.m_collision);

						const AABB hBox = AABB(aabb, Vector2f(_moveVec.x + sign(_moveVec.x), 0));
						if (hBox.intersects(bBox))
						{
							_actor->onTileCollision(cellX.tile.index, _moveVec, true, false);
							result.canMoveX = false;
						}
					}
				}
			}
		}

		for (int x = -k / 2; x < (k+1)/2; ++x)
		{
			for (uint y = 0; y < n; ++y)
			{
				const Vector2u cellPosY = m_level.getTilePos(pos + Vector2f(x * (int)m_levelTiles.m_size.x, sign(_moveVec.y) * m_levelTiles.m_size.y * y));
				if (isValidTilePos(cellPosY))
				{
					const CellInfo & cellY = m_level.get(cellPosY);
					const TileInfo & objectTileInfoY = m_levelTiles.m_tileInfos.get(m_levelTiles.indexToCoords(cellY.tile.index));

					if (isTileColliding(cellY.tile.index))
					{
						// ... vs Tile AABB
						Vector2f bPos = Vector2f(cellPosY.x * m_levelTiles.m_size.x + objectTileInfoY.m_origin.x, cellPosY.y * m_levelTiles.m_size.y + objectTileInfoY.m_origin.y);
						const AABB bBox(bPos, objectTileInfoY.m_collision);

						const AABB vBox = AABB(aabb, Vector2f(0, _moveVec.y + sign(_moveVec.y)));
						if (vBox.intersects(bBox))
						{
							_actor->onTileCollision(cellY.tile.index, _moveVec, false, true);
							result.canMoveY = false;
						}
					}
				}
			}
		}
	}

	// Check Tile Visuals
	if (_actor->m_isCollidingWithObjects)
	{
		for (int y = -k / 2; y < (k+1)/2; ++y)
		{
			for (uint x = 0; x < n; ++x)
			{
				const Vector2u cellPosX = m_level.getTilePos(pos + Vector2f(sign(_moveVec.x) * m_objectTiles.m_size.x * x, y * (int)m_objectTiles.m_size.y));
				if (isValidTilePos(cellPosX))
				{
					const CellInfo & cellX = m_level.get(cellPosX);
					const TileInfo & objectTileInfoX = m_objectTiles.m_tileInfos.get(m_objectTiles.indexToCoords(cellX.obj.index));

					const bool overlapZ = overlap(_actor->m_height, _actor->getTileInfo().m_collisionZ, 0.0f, objectTileInfoX.m_collisionZ);

					if (isTileObjectColliding(cellX.obj.index) && overlapZ)
					{
						// ... vs Tile AABB
						Vector2f bPos = Vector2f(cellPosX.x * m_objectTiles.m_size.x + 15 + cellX.obj.offset.x, cellPosX.y * m_objectTiles.m_size.y + 15 + cellX.obj.offset.y);
						const AABB bBox(bPos, objectTileInfoX.m_collision);

						const AABB hBox = AABB(aabb, Vector2f(_moveVec.x + sign(_moveVec.x), 0));
						if (hBox.intersects(bBox))
						{
							_actor->onObjectCollision(cellX.obj.index, _moveVec, true, false);
							result.canMoveX = false;
						}
					}
				}
			}
		}

		for (int x = -k/2; x < (k+1)/2; ++x)
		{
			for (uint y = 0; y < n; ++y)
			{
				const Vector2u cellPosY = m_level.getTilePos(pos + Vector2f(x * (int)m_objectTiles.m_size.x, sign(_moveVec.y) * m_objectTiles.m_size.y * y));
				if (isValidTilePos(cellPosY))
				{
					const CellInfo & cellY = m_level.get(cellPosY);
					const TileInfo & objectTileInfoY = m_objectTiles.m_tileInfos.get(m_objectTiles.indexToCoords(cellY.obj.index));

					const bool overlapZ = overlap(_actor->m_height, _actor->getTileInfo().m_collisionZ, 0.0f, objectTileInfoY.m_collisionZ);

					if (isTileObjectColliding(cellY.obj.index) && overlapZ)
					{
						// ... vs Tile AABB
						Vector2f bPos = Vector2f(cellPosY.x * m_objectTiles.m_size.x + 15 + cellY.obj.offset.x, cellPosY.y * m_objectTiles.m_size.y + 15 + cellY.obj.offset.y);
						const AABB bBox(bPos, objectTileInfoY.m_collision);

						const AABB vBox = AABB(aabb, Vector2f(0, _moveVec.y + sign(_moveVec.y)));
						if (vBox.intersects(bBox))
						{
							_actor->onObjectCollision(cellY.obj.index, _moveVec, false, true);
							result.canMoveY = false;
						}
					}
				}
			}
		}
	}

	return result;
}

//--------------------------------------------------------------------------
void SuperVimontBros::addRadZone(const RadZone & _radZone)
{
	m_radZones.push_back(_radZone);
}

//--------------------------------------------------------------------------
void SuperVimontBros::updateRadZones()
{
	auto zones = std::move(m_radZones);
	for (auto & zone : zones)
	{
		if (zone.life.getElapsedTime().asSeconds() < 10.0f)
		{
			m_radZones.push_back(zone);

			for (auto * player : m_players)
			{
				const float dist = length(player->getPosition() - zone.position);
				if (dist < zone.radius)
				{
					const auto & info = PlayerTypeInfo::get(player->getPlayerType());

					if (!(info.flags & PlayerTypeInfo::Flags::AtomicFart))
					{
						player->setRotten(true);
					}
				}
			}

			for (auto * bonus : m_bonuses.get())
			{
				if (!bonus->isRotten())
				{
					const float dist = length(bonus->getPosition() - zone.position);
					if (dist < zone.radius)
					{
						bonus->setRotten(true);
					}
				}
			}
		}
	}
}
