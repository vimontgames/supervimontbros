#pragma once

#include "Object/Object.h"
#include "Tiles/TileInfo.h"
#include "Physics/AABB.h"

using ShaderID = uint;

class Tiles;
class Viewport;

class Visual : public Object
{
public:
	Visual() {}
	Visual(const sf::String & _name, const Tiles & _tileSet);

	virtual void init();
	virtual void respawn();

	void setTileSet(const Tiles & _tileSet);

	void setShaderID(ShaderID _shaderID);
	void setLutIndex(u8 _lutIndex);

	void setImage(const sf::Vector2u & _coords, bool _flipX = false, bool _flipY = false);
	void setImage(uint _row, uint _column, bool _flipX = false, bool _flipY = false);

	void setColor(const sf::Color & _color);

	void setOrigin(const sf::Vector2f & _origin);
	const sf::Vector2f & getOrigin() const;

	void setPosition(const sf::Vector2f & _position);
	void setPosition(float _x, float _y);
	void setPosition(float _x, float _y, float _z);
	void setZ(float _z);

	void move(const sf::Vector2f & _offset);
	void move(float _x, float _y, float _z);

	const sf::Vector2f & getPosition() const { return m_sprite.getPosition(); }

	virtual void draw(sf::RenderTexture & _surface);
	virtual void drawShadow(sf::RenderTexture & _surface);

	virtual void updateAABB();
	virtual void update(const float _dt);

	const TileInfo & getTileInfo() const;
	static bool sortY(const Visual * _A, const Visual * _B);

	bool isImageSet() const { return m_coords.x != (uint)-1 && m_coords.y != (uint)-1; }

	const Tiles * getTileset() const { return m_tileSet; }

	void setSpawnCellPos(sf::Vector2u _cellPos);
	
//protected:
	ShaderID		m_shaderID = (ShaderID)-1;
	u8				m_lutIndex = 0;
	sf::Sprite		m_sprite;
	sf::Vector2u	m_coords = { (uint)-1, (uint)-1 };
	float			m_height = 0;
	float			m_angle = 0;
	const Tiles *	m_tileSet = nullptr;
	AABB			m_collisionAABB;
	AABB			m_visibilityAABB;
	bool			m_visible = true;
	sf::Vector2u	m_spawnCellPos;
	bool			m_isDecal = false;
};