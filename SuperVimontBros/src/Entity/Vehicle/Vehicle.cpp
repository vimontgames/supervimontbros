#include "Precomp.h"
#include "Vehicle.h"
#include "SuperVimontBros/SuperVimontBros.h"
#include "Entity/Player/Player.h"
#include "Entity/Enemy/Enemy.h"
#include "Entity/Ball/Ball.h"
#include "Entity/Shit/Shit.h"
#include "Entity/Bullet/Bullet.h"
#include "Controller/Controller.h"

using namespace sf;

//--------------------------------------------------------------------------
Vehicle::Vehicle(const sf::String & _name) :
	Entity(_name, Game::get().m_objectTiles)
{
	SuperVimontBros::get().m_vehicles.add(this);

	m_bounce = 0.5f;
	m_friction = 1.0f / 10000.0f;	
}

//--------------------------------------------------------------------------
Vehicle::~Vehicle()
{
	if (m_soundInstance)
	{
		m_soundInstance->stop();
	}
	removeWheels();	
	SuperVimontBros::get().m_vehicles.remove(this);
}

//--------------------------------------------------------------------------
void Vehicle::removeWheels()
{
	SuperVimontBros & game = SuperVimontBros::get();
	for (auto & wheel : m_wheels)
	{
		if (wheel.visual)
		{
			game.unregisterVisual(wheel.visual);
			SAFE_RELEASE(wheel.visual);
		}
	}
	m_wheels.clear();
}

//--------------------------------------------------------------------------
void Vehicle::init()
{
	Entity::init();

	addCustomSoundFX(SoundFX::Open, "SuperVimontBros/data/sound", "door.wav");
	addCustomSoundFX(SoundFX::Engine, "SuperVimontBros/data/sound", "engine.wav");
	addCustomSoundFX(SoundFX::Brake, "SuperVimontBros/data/sound", "brake.wav");
	addCustomSoundFX(SoundFX::Crash, "SuperVimontBros/data/sound", "crash.wav");
}

//--------------------------------------------------------------------------
void Vehicle::addWheel(const sf::Vector2u _image, sf::Vector2f _position)
{
	SuperVimontBros & game = SuperVimontBros::get();

	Visual * visual = new Visual("Wheel", game.m_spritesTile);
	m_wheels.push_back({ visual, _position });
	SAFE_INCREASE_REFCOUNT(visual);
	game.registerVisual(visual);

	visual->setImage(_image);
	visual->setPosition(getPosition() + _position);
	visual->init();
}

//--------------------------------------------------------------------------
void Vehicle::onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	if (Bullet * buller = dynamic_cast<Bullet*>(_other))
		return;

	if (Enemy * enemy = dynamic_cast<Enemy*>(_other))
	{
		if (m_velocity.x != 0 && !(enemy->getInfo().flags & EnemyTypeInfo::Flags::GoalKeeper))
		{
			Player * player = dynamic_cast<Player*>(getDriver());
			enemy->kill(player);
			return;
		}
	}

	if (Vehicle * vehicle = dynamic_cast<Vehicle*>(_other))
	{
		onVehicleCollision(vehicle);
	}

	if (Visual * visual = dynamic_cast<Visual*>(_other))
	{
		onVehicleCollision(visual);
	}

	Ball * ball = dynamic_cast<Ball*>(_other);
	Shit * shit = dynamic_cast<Shit*>(_other);
	if (ball || shit)
	{
		return;
	}

	Super::onActorCollision(_other, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
void Vehicle::onObjectCollision(objectIndex _objectIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	onVehicleCollision(nullptr);

	Super::onObjectCollision(_objectIndex, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
void Vehicle::onTileCollision(tileIndex _tileIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	if ((tileIndex)-1 != _tileIndex)
	{
		onVehicleCollision(nullptr);
	}

	Super::onObjectCollision(_tileIndex, _move, _horizontal, _vertical);
}

//--------------------------------------------------------------------------
bool Vehicle::onVehicleCollision(Visual * _other, bool _fromOtherVehicle)
{
	SuperVimontBros & game = SuperVimontBros::get();

	bool brokenWheel = false;

	if (abs(m_velocity.x) > 0.35f || _fromOtherVehicle)
	{
		bool front = false;

		if (_other)
		{
			front = getPosition().x < _other->getPosition().x;
		}
		else
		{
			front = m_velocity.x > 0;
		}

		if (front)
		{
			// Remove front wheel
			auto & info = m_wheels[1];
			if (nullptr != info.visual)
			{
				Game::get().ReleaseAsync(info.visual);
				info.visual = nullptr;
				playSound(SoundFX::Crash, 100);

				m_brokenWheels += 1;
				brokenWheel = true;

				for (auto & passenger : m_passengers)
				{
					if (Player * player = dynamic_cast<Player*>(passenger.entity))
					{
						player->m_hp = max(0, (int)player->m_hp - 4);
						player->playSound(SoundFX::Hit, 100);
					}
				}
			}
		}
		else
		{
			// Remove back wheel
			auto & info = m_wheels[0];
			if (nullptr != info.visual)
			{
				Game::get().ReleaseAsync(info.visual);
				info.visual = nullptr;
				playSound(SoundFX::Crash, 100);

				m_brokenWheels += 1;
				brokenWheel = true;

				for (auto & passenger : m_passengers)
				{
					if (Player * player = dynamic_cast<Player*>(passenger.entity))
					{
						player->m_hp = max(0, (int)player->m_hp - 2);
						player->playSound(SoundFX::Hit, 100);
					}
				}
			}
		}
	}

	if (brokenWheel)
	{
		if (m_brokenWheels == 1)
		{
			m_baseSpeedForward *= 0.75f;
			m_baseSpeedBackward *= 0.75f;
			m_turboSpeedForward = m_baseSpeedForward;
			m_turboSpeedBackward = m_baseSpeedBackward;
			m_maxSpeedForward *= 0.75f;
			m_maxSpeedBackward *= 0.75f;
		}
		else if (m_brokenWheels == m_wheels.size())
		{
			m_baseSpeedForward *= 0.1f;
			m_baseSpeedBackward *= 0.1f;
			m_turboSpeedForward = m_baseSpeedForward;
			m_turboSpeedBackward = m_baseSpeedBackward;
			m_maxSpeedForward *= 0.1f;
			m_maxSpeedBackward *= 0.1f;
		}

		if (!_fromOtherVehicle)
		{
			if (Vehicle * vehicle = dynamic_cast<Vehicle*>(_other))
			{
				vehicle->onVehicleCollision(this, true);
			}
		}

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
Entity * Vehicle::getDriver() const
{
	if (m_passengers.size() > 0)
	{
		return m_passengers[0].entity;
	}
	else
	{
		return nullptr;
	}
}

//--------------------------------------------------------------------------
void Vehicle::update(const float _dt)
{
	auto & game = SuperVimontBros::get();

	bool accelerate = false;
	bool brake = false;

	if (game.m_gameState == GameState::Play)
	{
		Player * driver = dynamic_cast<Player*>(getDriver());

		if (nullptr != driver)
		{
			const auto & pad = driver->getController();

			auto jDir = pad.getXYAxis();

			float jX = jDir.x;
			float jY = jDir.y;

			Vector2f joyDir = { 0,0 };

			const bool frontward = m_animLeft ? (jX < 0) : (jX > 0);

			float acceleration = frontward ? m_baseSpeedForward : m_baseSpeedBackward;

			if (pad.isButtonPressed((uint)Player::Button::Run))
			{
				// Accelerate
				acceleration = frontward ? m_turboSpeedForward : m_turboSpeedBackward;
				accelerate = true;
			}
			else if (pad.isButtonPressed((uint)Player::Button::Kick))
			{
				brake = true;
			}

			if (!brake)
			{
				if (jX > joyDeadZone)
				{
					joyDir += (Vector2f(1.0f, 0.0f) * acceleration);
				}
				else if (jX < -joyDeadZone)
				{
					joyDir += (-Vector2f(1.0f, 0.0f) * acceleration);
				}

				if (jY > joyDeadZone)
				{
					joyDir += (Vector2f(0.0f, 1.0f) * acceleration);
				}
				else if (jY < -joyDeadZone)
				{
					joyDir += (-Vector2f(0.0f, 1.0f) * acceleration);
				}

				joyDir.y = clamp(joyDir.y, -m_steering * abs(joyDir.x), m_steering * abs(joyDir.x));

				if (joyDir.x != 0 || joyDir.y != 0)
				{
					m_velocity += joyDir * _dt * 0.0001f;
				}
			}
		}

		float n = length(m_velocity);
		float maxSpeed;
		
		if (m_animLeft)
			maxSpeed = (m_velocity.x < 0) ? m_maxSpeedForward : m_maxSpeedBackward;
		else
			maxSpeed = (m_velocity.x > 0) ? m_maxSpeedForward : m_maxSpeedBackward;

		if (n > maxSpeed)
		{
			m_velocity *= maxSpeed / n; // clamp max speed
		}

		if (m_velocity.x != 0 || m_velocity.y != 0)
		{
			if (brake)
			{
				// Brake
				if (!isSoundPlaying((SoundIndex)SoundFX::Brake))
				{
					if (length(m_velocity) > 0.01f)
					{
						if (!isSoundPlaying((SoundIndex)SoundFX::Crash))
						{
							playSound(SoundFX::Brake, 50);
						}
					}
				}
				m_velocity *= clamp((1.0f - _dt / 250.0f), 0.0f, 1.0f);
			}
			else if (!accelerate)
			{
				// Slowdown
				//m_velocity *= clamp((1.0f - _dt / 2000.0f), 0.0f, 1.0f);
			}
		}
	}

	Entity::update(_dt);

	clampPositionToLevel(game.m_level);
	
	for (auto & passenger : m_passengers)
	{
		passenger.entity->setPosition(getPosition() + passenger.offset);
	}

	float soundVolume = 5 + clamp(length(m_velocity) * 10.0f, 0.0f, 95.0f);

	if (length(m_velocity) > 0.01f)
	{
		if (!isSoundPlaying((SoundIndex)SoundFX::Crash) && !isCurrentSound((SoundIndex)SoundFX::Engine) && !brake && getDriver())
		{
			playSound(SoundFX::Engine, 10, true);
		}
		setSoundVolume((int)soundVolume);
	}

	for (auto & wheel : m_wheels)
	{
		if (nullptr != wheel.visual)
		{
			wheel.visual->setPosition(getPosition() + wheel.pos);
			wheel.visual->m_angle += length(m_velocity) * sign(m_velocity.x) * 4.0f * _dt;
			wheel.visual->updateAABB();
		}
	}

	if (m_passengers.size() > 0)
	{
		auto list = findEntitiesInRadius(16);

		for (uint i = 0; i < list.size(); ++i)
		{
			Bonus * bonus = dynamic_cast<Bonus*>(list[i]);
			if (nullptr != bonus)
			{
				bool taken = false;

				for (auto & passenger : m_passengers)
				{
					Player * player = dynamic_cast<Player*>(passenger.entity);
					if (player)
					{
						if (player->onBonusHit(bonus, true))
						{
							taken = true;
						}
					}
				}

				if (m_passengers.size() > 0)
				{
					bonus->m_hp = 0;
				}
			}
		}

		for (auto & passenger : m_passengers)
		{
			addDebugText("%s\n", passenger.entity->name().toAnsiString().c_str());
		}

		addDebugText("%.2f\n", length(m_velocity));
	}
}

//--------------------------------------------------------------------------
bool Vehicle::addPassenger(Entity * _entity)
{
	if (m_addPassengerTimer.getElapsedTime().asMilliseconds() > 1000.0f)
	{
		playSound(SoundFX::Open);
		m_passengers.push_back({ _entity, _entity->getPosition() - getPosition() });
		m_addPassengerTimer.restart();
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Vehicle::removePassenger(Entity * _entity)
{
	if (m_addPassengerTimer.getElapsedTime().asMilliseconds() > 1000.0f)
	{
		auto it = std::find_if(m_passengers.begin(), m_passengers.end(), [_entity](const PassengerInfo & info) -> bool { return info.entity == _entity; });
		if (it != m_passengers.end())
		{
			m_passengers.erase(it);
		}
		playSound(SoundFX::Open);
		m_addPassengerTimer.restart();
		return true;
	}

	return false;
}