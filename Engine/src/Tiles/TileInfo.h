#pragma once

enum class TileFlags : u32
{
	None		= 0x00000000,
	Paint		= 0x00000001,
	Collide		= 0x00000002,
	Visual		= 0x00000004,
	Shadow		= 0x00000008
};
ENUM_FLAGS_OPERATORS(TileFlags);

struct TileInfo
{
	TileInfo()
	{
		m_box = { 0,0,0,0 };
		m_origin = { 15,15 };
		m_shadowOffset = 0;
		m_sortBias = 0;
		m_collisionZ = 16;
		m_flags = TileFlags::None;
	}

	bool hasCollisionBox() const
	{
		return m_collision.width != 0 && m_collision.height != 0;
	}

	sf::IntRect		m_box;
	sf::IntRect		m_collision;
	sf::Vector2i	m_origin;
	i8				m_shadowOffset;
	i8				m_sortBias;
	Vector2i8		m_hotSpot;
	u8				m_collisionZ;
	i8				_pad[15];
	TileFlags		m_flags;
};