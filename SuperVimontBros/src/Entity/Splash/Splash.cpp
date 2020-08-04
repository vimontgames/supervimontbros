#include "Precomp.h"
#include "Splash.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Tiles/LevelObject.h"
#include "Entity/Enemy/Enemy.h"

using namespace sf;

//--------------------------------------------------------------------------
Splash::Splash(SplashType _splashType, uint _subIndex) :
	Entity("Splash", Game::get().m_objectTiles),
	m_splashType(_splashType), m_subIndex((uint)_subIndex)
{
	m_lifeTime = 15000;
	m_isDecal = true;
}

//--------------------------------------------------------------------------
Splash::~Splash()
{
	
}

//--------------------------------------------------------------------------
void Splash::init()
{
	Entity::init();

	Vector2u splashImg;

	switch (m_splashType)
	{
		default:
			assert(false);

		case SplashType::Cinder:
			splashImg = m_tileSet->indexToCoords((uint)LevelObject::CinderSplash);
			break;

		case SplashType::PlayerThrowUp:
			splashImg = m_tileSet->indexToCoords((uint)LevelObject::PlayerThrowUpSplash);
			break;

		case SplashType::Enemy:
			switch ((EnemyType)m_subIndex)
			{
			default:
				assert(false);
				
			case EnemyType::ZombiePatrick:
				splashImg = m_tileSet->indexToCoords((uint)LevelObject::PatrickSplash);
				break;

				case EnemyType::Zombie:
					splashImg = m_tileSet->indexToCoords((uint)LevelObject::ZombieSplash);
					break;

				case EnemyType::Footballer:
					splashImg = m_tileSet->indexToCoords((uint)LevelObject::FootballerSplash);
					break;

				case EnemyType::Footballer2:
					splashImg = m_tileSet->indexToCoords((uint)LevelObject::Footballer2Splash);
					break;

				case EnemyType::Goalkeeper:
					splashImg = m_tileSet->indexToCoords((uint)LevelObject::GoalkeeperSplash);
					break;

				case EnemyType::Goalkeeper2:
					splashImg = m_tileSet->indexToCoords((uint)LevelObject::Goalkeeper2Splash);
					break;

				case EnemyType::Rugbyman:
					splashImg = m_tileSet->indexToCoords((uint)LevelObject::RugbymanSplash);
					break;

				case EnemyType::YellowVest:
					splashImg = m_tileSet->indexToCoords((uint)LevelObject::YellowVestSplash);
					break;

				case EnemyType::ZombiePlaya:
					splashImg = m_tileSet->indexToCoords((uint)LevelObject::ZombiePlayaSplash);
					break;

				case EnemyType::ZombieCowboy:
					splashImg = m_tileSet->indexToCoords((uint)LevelObject::ZombieCowboySplash);
					break;
			}
			
			break;
	}

	AnimationSequence & idle = getAnimationSequence(Animation::Idle);
	idle.addFrame(AnimFrame(splashImg));
}

//--------------------------------------------------------------------------
void Splash::update(const float _dt)
{
	const uint t = m_animTimer.getElapsedTime().asMilliseconds();

	if (t > m_lifeTime)
	{
		fadeOutAndKill(500);
	}

	Entity::update(_dt);

	if (t <= 500)
	{
		uint fade = t * 255 / 500;
		const Color & color = m_sprite.getColor();
		setColor(Color(color.r, color.g, color.b, fade));
	}

	playAnimation(Animation::Idle, m_animLeft);
}