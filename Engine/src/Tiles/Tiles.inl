//--------------------------------------------------------------------------
INLINE sf::Vector2u Tiles::count() const
{
	return m_tileInfos.size();
}

//--------------------------------------------------------------------------
INLINE sf::Vector2u Tiles::indexToCoords(uint _index) const
{
	return indexToCoords(_index, count().x);
}

//--------------------------------------------------------------------------
INLINE sf::Vector2u Tiles::indexToCoords(uint _index, u32 _width) const
{
	const u32 row = _index / _width;
	const u32 column = _index - row * _width;
	return { column, row };
}

//--------------------------------------------------------------------------
INLINE uint Tiles::coordsToIndex(const sf::Vector2u & _coords) const
{
	return _coords.x + _coords.y * count().x;
}