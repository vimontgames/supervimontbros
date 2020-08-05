#include "Precomp.h"
#include "Visual.h"
#include "Game/Game.h"
#include "Shader/Shader.h"

using namespace sf;

//--------------------------------------------------------------------------
Visual::Visual(const String & _name, const Tiles & _tileSet) :
	Object(_name),
	m_tileSet(&_tileSet),
	m_collisionAABB({ 0,0 }, { 0,0,0,0 })
{
	m_sprite.setTextureRect(IntRect(0, 0, 1, 1));
	m_sprite.setTexture(_tileSet.m_texture);
	m_sprite.setColor(Color(255, 255, 255, 255));
}

//--------------------------------------------------------------------------
void Visual::init()
{
	updateAABB();
}

//--------------------------------------------------------------------------
void Visual::respawn()
{
	init();
}

//--------------------------------------------------------------------------
const TileInfo & Visual::getTileInfo() const
{
	return m_tileSet->m_tileInfos.get(m_coords);
}

//--------------------------------------------------------------------------
bool Visual::sortY(const Visual * _A, const Visual * _B)
{
	return (_A->m_sprite.getPosition().y + _A->getTileInfo().m_sortBias /*+ _A->m_height*/) < (_B->m_sprite.getPosition().y + _B->getTileInfo().m_sortBias /*+ _B->m_height*/);
}

//--------------------------------------------------------------------------
void Visual::setTileSet(const Tiles & _tileSet)
{
	m_tileSet = &_tileSet;
}

//--------------------------------------------------------------------------
void Visual::setImage(const Vector2u & _coords, bool _flipX, bool _flipY)
{
	m_coords = _coords;

	const TileInfo & info = getTileInfo();

	m_sprite.setTexture(m_tileSet->m_texture);
	m_sprite.setTextureRect(m_tileSet->getTileRect(_coords, _flipX, _flipY));
	m_sprite.setOrigin(-Vector2f(info.m_box.left, info.m_box.top));
}

//--------------------------------------------------------------------------
void Visual::setImage(uint _row, uint _column, bool _flipX, bool _flipY)
{
	setImage({ _row, _column }, _flipX, _flipY);
}

//--------------------------------------------------------------------------
void Visual::setColor(const sf::Color & _color)
{
	m_sprite.setColor(_color);
}

//--------------------------------------------------------------------------
const sf::Color & Visual::getColor() const
{
	return m_sprite.getColor();
}

//--------------------------------------------------------------------------
void Visual::setPosition(const Vector2f & _position)
{
	m_sprite.setPosition(_position);
}

//--------------------------------------------------------------------------
void Visual::setPosition(float _x, float _y)
{
	m_sprite.setPosition({ _x, _y });
}

//--------------------------------------------------------------------------
void Visual::setPosition(float _x, float _y, float _z)
{
	setPosition({ _x, _y });
	m_height = _z;
}

//--------------------------------------------------------------------------
void Visual::setZ(float _z)
{
	m_height = _z;
}

//--------------------------------------------------------------------------
void Visual::setOrigin(const Vector2f & _origin)
{
	m_sprite.setOrigin(_origin);
}

//--------------------------------------------------------------------------
const Vector2f & Visual::getOrigin() const
{
	return m_sprite.getOrigin();
}

//--------------------------------------------------------------------------
void Visual::move(const sf::Vector2f & _offset)
{
	m_sprite.move(_offset);
}

//--------------------------------------------------------------------------
void Visual::move(float _x, float _y, float _z)
{
	m_sprite.move({ _x, _y });
	m_height += _z;
}

//--------------------------------------------------------------------------
void Visual::update(const float _dt)
{

}

//--------------------------------------------------------------------------
void Visual::updateAABB()
{
	if (m_coords.x != -1 && m_coords.y != -1)
	{
		// check collisions		
		const TileInfo & info = m_tileSet->m_tileInfos.get(m_coords);

		// Player AABB ...
		Vector2f aPos = getPosition();
		m_collisionAABB = AABB(aPos, info.m_collision);

		// Shadow AABB
		m_visibilityAABB = AABB(aPos, IntRect(info.m_box.left, info.m_box.top - m_height, info.m_box.width, info.m_box.height + info.m_box.height/2 + m_height));
	}
}

//--------------------------------------------------------------------------
void Visual::setShaderID(ShaderID _shaderID)
{
	assert(invalidShaderID != _shaderID);
	m_shaderID = _shaderID;
}

//--------------------------------------------------------------------------
void Visual::setLutIndex(u8 _lutIndex)
{
	assert(_lutIndex < Game::get().m_spriteLutCount);
	m_lutIndex = _lutIndex;
}

//--------------------------------------------------------------------------
void Visual::setSpawnCellPos(sf::Vector2u _cellPos)
{
	m_spawnCellPos = _cellPos;
}

//--------------------------------------------------------------------------
void Visual::draw(sf::RenderTexture & _surface)
{
	if (!m_visible || m_sprite.getColor().a == 0)
		return;

	Sprite sprite = m_sprite;
		   sprite.setPosition((int)m_sprite.getPosition().x, (int)m_sprite.getPosition().y - m_height);

	if (m_angle != 0)
	{
		sprite.setRotation(m_angle);
	}

	if (invalidShaderID == m_shaderID)
	{
		_surface.draw(sprite);
	}
	else
	{
		auto & game = Game::get();
		sf::Shader * shader = ::Shader::get(m_shaderID);

		shader->setUniform("texture", m_tileSet->m_texture);
		shader->setUniform("lut", game.m_spriteLut);
		shader->setUniform("lutIndex", (int)m_lutIndex);

		_surface.draw(sprite, shader);
	}

	const Game & game = Game::get();

	if (game.m_debugDisplay)
	{
		if (game.m_drawCollisions)
		{
			if (m_tileSet == &game.m_objectTiles)
			{
				const objectIndex index = m_coords.x + m_tileSet->m_tileInfos.size().x * m_coords.y;
				if (game.isTileObjectColliding(index))
				{
					m_collisionAABB.draw(_surface, { 187,255,187,128 });
					//m_visibilityAABB.draw(_surface, { 187,255,187, 128 });
				}
			}
			else if (m_tileSet == &game.m_spritesTile)
			{
				m_collisionAABB.draw(_surface, { 255,187,187,128 });
				//m_visibilityAABB.draw(_surface, { 255,187,187, 128 });
			}
		}
	}
}

//--------------------------------------------------------------------------
void Visual::drawShadow(sf::RenderTexture & _surface)
{
	if (!m_visible)
		return;
	
	const TileInfo & info = getTileInfo();
	Sprite shadow = m_sprite;
		   shadow.setOrigin(m_sprite.getOrigin().x, 0);
		   shadow.setPosition((int)m_sprite.getPosition().x, (int)m_sprite.getPosition().y);
		   //shadow.setColor({ 0,0,0, (u8)(m_sprite.getColor().a / 4) });
		   shadow.setScale({ 1,-0.5f});
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