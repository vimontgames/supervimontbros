//--------------------------------------------------------------------------
INLINE sf::Vector2u Level::count() const
{ 
	return m_cells.size(); 
}

//--------------------------------------------------------------------------
INLINE void Level::setTile(const sf::Vector2u & _coords, tileIndex _index)
{
	CellInfo info = m_cells.get(_coords);
	info.tile.index = _index;

	m_cells.set(_coords, info);
}

//--------------------------------------------------------------------------
INLINE void Level::setDecal(const sf::Vector2u & _coords, decalIndex _index)
{
	CellInfo info = m_cells.get(_coords);
	info.decal.index = _index;

	m_cells.set(_coords, info);
}

//--------------------------------------------------------------------------
INLINE void Level::setObject(const sf::Vector2u & _coords, objectIndex _index)
{
	CellInfo info = m_cells.get(_coords);
	info.obj.index = _index;

	m_cells.set(_coords, info);
}

//--------------------------------------------------------------------------
INLINE const CellInfo & Level::get(const sf::Vector2u & _coords) const
{
	return m_cells.get(_coords);
}