#pragma once

#include"Visual/Visual.h"
#include "Array2D/Array2D.h"

class Viewport;

typedef u16 tileIndex;
typedef u16 decalIndex;
typedef u16 objectIndex;

enum class CellFlags : u8
{
	None	= 0x0000,
	FlipX	= 0x0001,
	FlipY	= 0x0002
};
ENUM_FLAGS_OPERATORS(CellFlags);

template <class T> struct CellIndex
{
	CellIndex() :
		index(0), offset({ 0,0 }), palette(0), flags(CellFlags::None)
	{
	}

	T			index;
	Vector2i8	offset;
	CellFlags	flags;
	u8			palette;
};

enum class CellDataIndex : u8
{
	Tile = 0,
	Decal,
	Object,

	Count
};

struct CellInfo
{
	CellInfo() : tile(), decal(), obj()
	{
	}

	union
	{
		struct
		{
			CellIndex<tileIndex> tile;
			CellIndex<decalIndex> decal;
			CellIndex<objectIndex> obj;
		};
		CellIndex<u16> data[(uint)CellDataIndex::Count];
	};
};

enum class BgTileCategory
{
	Ground = 0,
	Decal,
	Shadow,

	Count
};

enum class TilesType : u8;

class Level 
{
public:
	Level() {}
	
	bool init(sf::String _filename, const sf::Vector2u & _count, const Tiles * _backgroundTiles, const Tiles * _decalTiles, const Tiles * _objectTiles);
	bool update();

	bool load();
	bool save();

	bool draw(Viewport & _viewport, BgTileCategory _category);
	bool drawTilesCollisions(Viewport & _viewport);
	bool drawObjectsCollisions(Viewport & _viewport);
	bool drawObjects(Viewport & _viewport);

	bool drawPainting(sf::RenderTexture & _surface);

	bool resize(const sf::Vector2u & _count);

	// copy/pasta
	void copySelectedRect(const sf::Vector2i & _min, const sf::Vector2i & _max, bool _cut);

	const sf::Vector2u getTilePos(const sf::Vector2f & _position) const;
	const CellInfo & getTileUnderPos(const sf::Vector2f & _position) const;

	sf::Vector2u count() const;
	const CellInfo & get(const sf::Vector2u & _coords) const;

private:
	bool updatePainting();
	void buildTilesVertexArray();

	void setTile(const sf::Vector2u & _coords, tileIndex _index);
	void setDecal(const sf::Vector2u & _coords, decalIndex _index);
	void setObject(const sf::Vector2u & _coords, objectIndex _index);

	static void drawCellTile(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, Visual & _visual, u32 _x, u32 _y, u32 _opacity = 255);
	static void drawCellDecal(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, Visual & _visual, u32 _x, u32 _y, u32 _opacity = 255);
	static void drawCellObject(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, Visual & _visual, u32 _x, u32 _y, u32 _opacity = 255);

	void drawCellTileCollision(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, u32 _x, u32 _y, u32 _opacity = 255);
	void drawObjectCollision(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, u32 _x, u32 _y, u32 _opacity = 255);

	void getVisibleTilesRange(sf::Vector2u & _min, sf::Vector2u & _max, const Tiles & _tiles, const Viewport & _viewport) const;
	uint getBgTileCategoryMask(const CellInfo & _cell) const;

	template <class T> static void drawCellInternal(sf::RenderTexture & _surface, const Tiles & _tiles, const T & _cell, Visual & _visual, u32 _x, u32 _y, u32 _opacity = 255);

	const Tiles * getCurrentTileSet() const;
	CellDataIndex getCurrentCellInfoIndex() const;

	bool onTileSetResized(const Tiles * _tileSet, const sf::Vector2u _previousSize);

public:
	sf::String						m_filename;
	Array2D<CellInfo>				m_cells;

	const Tiles *					m_backgroundTiles = nullptr;
	const Tiles *					m_decalTiles = nullptr;
	const Tiles *					m_objectTiles = nullptr;

	sf::Vector2u					m_mouseOverTile = sf::Vector2u(-1, -1);
	sf::Vector2u					m_selectedTile = sf::Vector2u(0, 0);
	sf::Vector2f					m_paintCamera = sf::Vector2f(0, 0);
	bool							m_canPaint = false;
	std::vector<Visual>				m_visuals;
	Array2D<CellInfo>				m_selectedCopy;

	sf::VertexArray					m_tilesVertexArray[(uint)BgTileCategory::Count];
	bool							m_tilesVertexArrayDirty = true;
};

#ifdef ENABLE_INL
#include "Level.inl"
#endif