#pragma once

#include "Array2D/Array2D.h"
#include "TileInfo.h"

enum class TilesType : u8
{
	LevelTiles = 0,
	ObjectTiles,
	DecalTiles,
	SpriteTiles,

	Count
};

enum class CellDataIndex : u8;

class Level;

class Tiles
{
public:

	bool init(sf::String _texturePath, const sf::Vector2u & _size, const sf::Vector2u &_count, TilesType _type, bool _reinit = false);
	bool update();

	bool updateSelectTiles();

	bool drawSelectTiles(sf::RenderTexture & _surface);

	bool isValidTileSelected() const;

	sf::IntRect getTileRect(const sf::Vector2u & _coords, bool _flipX = false, bool _flipY = false) const;
	uint getSelectedTileIndex() const;

	sf::Vector2u count() const;
	sf::Vector2u indexToCoords(uint _index) const;
	sf::Vector2u indexToCoords(uint _index, u32 _width) const;
	uint coordsToIndex(const sf::Vector2u & _coords) const;

	bool load();
	bool save();

	sf::Color getColor() const;
	sf::Color getTransparentColor() const;
	sf::Color getBackgroundColor() const;

	const char *		getTileTypeName() const;
	CellDataIndex		getCellInfoIndex() const;

	void				updateTilePresentationOffset();

	sf::String			m_tileInfoFilename;
	TilesType			m_tilesType;

	sf::Texture			m_texture;
	sf::Vector2u		m_size;
	sf::Vector2u		m_mouseOverTile = sf::Vector2u((uint)-1, (uint)-1);
	sf::Vector2u		m_selectedTile = sf::Vector2u((uint)0, (uint)0);

	Array2D<TileInfo>	m_tileInfos;
	bool				m_editTiles = false;
	sf::Vector2f		m_selectTilesOffset = { 0, 40 };
	sf::Vector2f		m_tilePresentationOffset = { 0, 0 };

	bool				m_drawShadow = false;
	bool				m_drawOrigin = false;
	bool				m_drawHotSpot = false;
	bool				m_drawBox = false;
	bool				m_drawCollision = false;
	bool				m_drawBias = false;
	bool				m_drawFlags = false;
	bool				m_drawHeight = false;

	TileInfo			m_copyPaste;
	bool				m_copySource = false;
};

#ifdef ENABLE_INL
#include "Tiles.inl"
#endif