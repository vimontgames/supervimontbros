#include "Precomp.h"
#include "Actor.h"
#include "Game/Game.h"
#include "Sound/SoundBank.h"
#include "Shader/Shader.h"

#ifndef ENABLE_INL
#include "Actor.inl"
#endif

using namespace sf;

//--------------------------------------------------------------------------
Actor::Actor(const String & _name, const Tiles & _tileSet) :
	Visual(_name, _tileSet)
{
	m_isColliderForOtherActors = true;
}

//--------------------------------------------------------------------------
Actor::~Actor()
{
	if (m_soundInstance)
	{
		SoundBank::get().releaseSoundInstance(m_soundInstance);
	}

	SAFE_FREE(m_debugBuffer);
	SAFE_RELEASE(m_parent);
}

//--------------------------------------------------------------------------
void Actor::onActorCollision(Actor * _other, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	if (_horizontal)
	{
		_move.x = -m_bounce * _move.x;
	}

	if (_vertical)
	{
		_move.y = -m_bounce * _move.y;
	}
}

//--------------------------------------------------------------------------
void Actor::onBeingCollided(Actor * _byActor, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{

}

//--------------------------------------------------------------------------
void Actor::onObjectCollision(objectIndex _objectIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	if (_horizontal)
	{
		_move.x = -m_bounce * _move.x;
	}

	if (_vertical)
	{
		_move.y = -m_bounce * _move.y;
	}
}

//--------------------------------------------------------------------------
void Actor::onTileCollision(tileIndex _tileIndex, sf::Vector2f & _move, bool _horizontal, bool _vertical)
{
	if (_horizontal)
	{
		_move.x = -m_bounce * _move.x;
	}

	if (_vertical)
	{
		_move.y = -m_bounce * _move.y;
	}
}

//--------------------------------------------------------------------------
void Actor::draw(RenderTexture & _surface)
{
	Game & game = Game::get();

	if (game.m_editorMode == EditorMode::None)
	{
		Visual::draw(_surface);

		if (Game::get().m_debugDisplay)
		{
			sf::Text dbgText;

			dbgText.setString(m_debugBuffer);
			dbgText.setFont(Game::get().getFont(GameFont::Dlx8));
			dbgText.setCharacterSize(8);
			dbgText.setFillColor({ 255,255,255,128 });

			Vector2f pos = getPosition() + Vector2f(-dbgText.getLocalBounds().width * 0.5f, 4);

			pos.x = (int)pos.x;
			pos.y = (int)pos.y;			

			dbgText.setPosition(pos);

			_surface.draw(dbgText);
		}
	}	
}

//--------------------------------------------------------------------------
void Actor::init()
{
	Visual::init();

	//m_faceLeft = false;
	m_animTimer.restart();
	
	m_animSequences.clear();
	m_animSequences.reserve(maxAnimIndex);

	for (uint i = 0; i < maxSoundIndex; ++i)
	{
		m_soundIndex[i] = -1;
	}

	m_curSoundIndex = invalidSoundIndex;

	if (m_soundInstance)
	{
		SoundBank::get().releaseSoundInstance(m_soundInstance);
	}
}

//--------------------------------------------------------------------------
AnimationSequence & Actor::getAnimationSequence(AnimIndex _animIndex)
{
	if ((uint)_animIndex >= m_animSequences.size())
	{
		m_animSequences.resize((uint)_animIndex +1);
	}
	return m_animSequences[(uint)_animIndex];
}

//--------------------------------------------------------------------------
const AnimationSequence & Actor::getAnimationSequence(AnimIndex _animIndex) const
{
	return m_animSequences[(uint)_animIndex];
}

//--------------------------------------------------------------------------
const AnimationSequence & Actor::getCurrentAnimationSequence() const
{
	return getAnimationSequence(m_animIndex);
}

//--------------------------------------------------------------------------
void Actor::update(const float _dt)
{
	clearDebugText();

	Game & game = Game::get();

	m_firstFrame = false;
	m_previousPos = getPosition();

	if (isFadedOut() && !m_respawn)
	{
		Game::get().ReleaseAsync(this);
		this->release();
		return;
	}

	const float norm = length(m_velocity);

	if (abs(norm) > 0.00001f)
	{
		Vector2f delta = m_velocity * _dt;
		if (game.tryMove(this, delta))
		{
			move(delta);
			const float eps = 0.00001f * _dt;
			m_velocity.x = abs(m_velocity.x) > eps ? abs(m_velocity.x) * sign(m_velocity.x) * (delta.x / (m_velocity.x * _dt)) : 0.0f;
			m_velocity.y = abs(m_velocity.y) > eps ? abs(m_velocity.y) * sign(m_velocity.y) * (delta.y / (m_velocity.y * _dt)) : 0.0f;
		}

		m_velocity.x = delta.x != 0.0f ? m_velocity.x : 0.0f;
		m_velocity.y = delta.y != 0.0f ? m_velocity.y : 0.0f;
	}

	if (abs(m_jump ) > 0.000001f || abs(m_height) > 0.000001f)
	{
		m_height += _dt * m_jump;

		if (m_height < 0)
		{
			m_height = 0;
			m_jump *= -m_bounce * m_bounce;
			m_height += _dt * m_jump;
		}
		else
		{
			m_jump -= _dt * 9.81f * 0.00002f;
		}
	}
	else
	{
		m_jump = 0.0f;
		m_height = 0.0f;
	}

	float deltaAngle = min(abs(norm * 360.0f * length(m_velocity) * 1.0f), 1.0f);

	if (m_rotateWhenMoving && deltaAngle > 0.01f)
	{
		if (m_velocity.x < 0)
		{
			m_angle -= deltaAngle * _dt;
		}
		else
		{
			m_angle += deltaAngle * _dt;
		}
	}

	m_velocity *= (1.0f - _dt * m_friction);

	if (norm < 0.0001f)
	{
		m_velocity = { 0,0 };
	}

	Visual::update(_dt);

	if (invalidAnimIndex != m_animIndex)
	{
		const Tiles & tiles = *getTileset();
		
		const AnimationSequence & sequence = getAnimationSequence(m_animIndex);
		const uint count = (uint)sequence.size();

		if (count > 0)
		{
			uint t = m_animTimer.getElapsedTime().asMilliseconds();
			t = t % sequence.m_totalDuration;

			uint duration = 0;
			for (uint i = 0; i < count; ++i)
			{
				const AnimFrame & frame = sequence[i];
				duration += frame.m_duration;

				if (t <= duration)
				{
					Vector2u coords = frame.m_imageCoords;

					if (m_hasAnimLeft)
					{
						if (m_animLeft)
						{
							coords.y += 1;
						}
					}
					else
					{
						const auto & scale = m_sprite.getScale();
						if (m_animLeft && scale.x > 0)
						{
							m_sprite.setScale({ -scale.x, scale.y });
						}
						else if (!m_animLeft && m_sprite.getScale().x < 0)
						{
							m_sprite.setScale({ -scale.x, scale.y });
						}
					}

					setImage(coords);

					m_animFrame = i;

					break;
				}
			}
		}
	}

	updateAABB();

#if 0
	if (game.m_debugDisplay)
	{
		char debugString[256] = { '\0' };
		
		if (-1 != m_animFrame)
		{
			sprintf_s(debugString,	"%u %i\n", m_animIndex, m_animFrame);
		}
		
		m_debugText.setString(String(debugString));
	}
#endif
}

//--------------------------------------------------------------------------
void Actor::clearDebugText()
{
	m_debugBuffer = '\0';
}

#define DEBUGBUFFER_SIZE 1024

//--------------------------------------------------------------------------
void Actor::addDebugText(const char * _format, ...)
{
	if (Game::get().m_debugDisplay)
	{
		va_list args;
		va_start(args, _format);
		char buffer[4096];
		vsnprintf(buffer, sizeof(buffer), _format, args);
		if (!m_debugBuffer)
		{
			m_debugBuffer = (char*)malloc(sizeof(char)*DEBUGBUFFER_SIZE);
			m_debugBuffer[0] = '\0';
		}
		strcat_s(m_debugBuffer, DEBUGBUFFER_SIZE, buffer);
		va_end(args);
	}
}

//--------------------------------------------------------------------------
//String Actor::getAnimationName(AnimIndex _animation) const
//{
//	switch (_animation)
//	{
//		case Animation::Idle:
//			return "Attente";
//
//		case Animation::Run:
//			return "Course";
//
//		case Animation::Walk:
//			return "Marche";
//
//		case Animation::Punch:
//			return "Poing";
//
//		case Animation::PrepareKick:
//			return "Prepare";
//
//		case Animation::Kick:
//			return "Shoot";
//
//		case Animation::Celebrate:
//			return "Youpi";
//
//		case Animation::Die:
//			return "Arg";
//
//		default:
//			return "";
//	}
//}

//--------------------------------------------------------------------------
bool Actor::addSound(SoundIndex _sound, const char * _fullpath)
{
	m_soundIndex[(u32)_sound] = SoundBank::get().addSound(_fullpath);
	return -1 != m_soundIndex[(u32)_sound];
}

//--------------------------------------------------------------------------
bool Actor::addCustomSound(SoundIndex _sound, const char * _path, const char * _filename)
{
	char path[256];
	sprintf_s(path, "%s/%s/%s", _path, name().toAnsiString().c_str(), _filename);
	return addSound(_sound, path);
}

//--------------------------------------------------------------------------
sf::Sound * Actor::getSoundInstance()
{
	if (!m_soundInstance)
	{
		m_soundInstance = SoundBank::get().getSoundInstance();
	}

	return m_soundInstance;
}

//--------------------------------------------------------------------------
bool Actor::playSound(SoundIndex _sound, int _volume, bool _loop)
{
	int index = m_soundIndex[(u32)_sound];
	if (-1 != index)
	{
		auto & entry = SoundBank::get().getSound(index);
		auto * sound = getSoundInstance();

		sound->setBuffer(entry.buffer);
		sound->setVolume(_volume);
		sound->play();
		sound->setLoop(_loop);

		m_soundTimer.restart();
		m_curSoundIndex = _sound;
		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------------------
void Actor::setSoundVolume(int _volume)
{
	auto * sound = getSoundInstance();
	sound->setVolume(_volume);
}

//--------------------------------------------------------------------------
bool Actor::isCurrentSound(SoundIndex _sound) const
{
	return m_curSoundIndex == _sound;
}

//--------------------------------------------------------------------------
bool Actor::isSoundPlaying(SoundIndex _sound) const
{
	int index = m_soundIndex[(u32)_sound];
	if (-1 != index)
	{
		float t = m_soundTimer.getElapsedTime().asMilliseconds();
		float d = SoundBank::get().getSound(index).buffer.getDuration().asMilliseconds();
		return isCurrentSound(_sound) && t < d;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------------------
sf::String Actor::getSoundName(SoundIndex _sound)
{
	static const char * soundNames[] =
	{
		"Grrr"	// Grrr
	};

	return soundNames[(u32)_sound];
}

//--------------------------------------------------------------------------
bool Actor::playAnim(AnimIndex _animIndex, bool _left, bool _restart)
{
	const AnimationSequence & sequence = getAnimationSequence(_animIndex);
	if (sequence.size() == 0)
	{
		debugPrint("Animation %u does not exist for actor \"%s\"\n", m_animIndex, name().toAnsiString().c_str());
		return false;
	}

	if (m_animIndex != _animIndex || m_animLeft != _left)
	{
		m_animIndex = _animIndex;
		m_animLeft = _left;
		m_animTimer.restart();
	}
	else if (_restart)
	{
		m_animTimer.restart();
	}

	return true;
}

//--------------------------------------------------------------------------
void Actor::fadeOutAndKill(uint _ms)
{
	if (m_fading == false)
	{
		m_fading = true;
		m_fadeOut = true;
		m_fadeStartTime = m_lifeTime.getElapsedTime().asMilliseconds();
		m_fadeDuration = _ms;
	}
}

//--------------------------------------------------------------------------
bool Actor::isFadedOut()
{
	uint t = m_lifeTime.getElapsedTime().asMilliseconds();
	const auto & color = m_sprite.getColor();

	if (m_waitRespawn)
	{
		if (m_respawnTimer.getElapsedTime().asSeconds() > m_respawnTime)
		{
			m_fading = true;
			m_fadeIn = true;
			m_fadeOut = false;
			m_waitRespawn = false;
			m_lifeTime.restart();
			respawn();
			t = 0;
		}
	}

	if (m_fading)
	{
		if (m_fadeOut)
		{
			uint diff = min(t - m_fadeStartTime, m_fadeDuration);
			uint opacity = (diff*255.0f) / (float)m_fadeDuration;
			setColor(Color(color.r, color.g, color.b, 255 - opacity));

			if (t - m_fadeStartTime > m_fadeDuration)
			{
				m_fading = false;
				m_fadeOut = true;
				m_fadeIn = false;

				m_respawnTimer.restart();
				m_waitRespawn = true;

				return true;
			}
		}
		else if (m_fadeIn)
		{
			float opacity = min(255.0f, (t*255.0f) / (float)m_fadeDuration);
			setColor(Color(color.r, color.g, color.b, (u8)opacity));

			if (opacity >= 255.0f)
			{
				m_fading = false;
				m_fadeIn = false;
			}
		}
	}

	return false;
}

//--------------------------------------------------------------------------
void Actor::drawShadow(sf::RenderTexture & _surface)
{
	if (!m_visible)
		return;

	const TileInfo & info = getTileInfo();
	Sprite shadow = m_sprite;
	shadow.setOrigin(m_sprite.getOrigin().x, 0);
	shadow.setPosition((int)m_sprite.getPosition().x, (int)m_sprite.getPosition().y);
	//shadow.setColor({ 0,0,0, (u8)(m_sprite.getColor().a / 4) });

	if (m_animLeft && !m_hasAnimLeft)
		shadow.setScale({ -1.0f,-0.5f });
	else
		shadow.setScale({ 1.0f,-0.5f });

	shadow.move(0.0f, info.m_shadowOffset + m_tileSet->m_size.y / 2);

	auto & game = Game::get();

	auto * shader = ::Shader::get(game.m_shadowShader);
		   shader->setUniform("texture", m_tileSet->m_texture);
		   shader->setUniform("instance", true);

	sf::RenderStates rs;
					 rs.shader = shader;
					 rs.blendMode = game.m_shadowBlendMode;

	_surface.draw(shadow, rs);
}