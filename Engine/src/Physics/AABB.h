#pragma once 

class AABB
{
public:
	AABB(sf::Vector2f _pos = { 0,0 }, sf::Vector2f _half = { 0,0 }) :
		m_pos(_pos),
		m_half(_half)
	{

	}

	AABB(const AABB & _other) :
		m_pos(_other.m_pos),
		m_half(_other.m_half)
	{

	}

	AABB(sf::Vector2f _pos, sf::IntRect _rect) :
		m_pos({ _pos.x + (float)_rect.width * 0.5f + (float)_rect.left, _pos.y + (float)_rect.height * 0.5f + (float)_rect.top }),
		m_half({ (float)_rect.width * 0.5f , (float)_rect.height * 0.5f })
	{

	}

	AABB(const AABB & _other, const sf::Vector2f & _offset) :
		m_half(_other.m_half),
		m_pos(_other.m_pos + _offset)
	{
		
	}

	bool intersects(const AABB & _other) const
	{
		sf::Vector2f hitPos, normal;
		return intersects(_other, hitPos, normal);
	}

	bool intersects(const AABB & _other, sf::Vector2f & _hitPos, sf::Vector2f & _normal) const
	{
		const float dx = _other.m_pos.x - m_pos.x;
		const float dy = _other.m_pos.y - m_pos.y;

		const float px = (_other.m_half.x + m_half.x) - abs(dx);
		const float py = (_other.m_half.y + m_half.y) - abs(dy);

		if (px <= 0 || py <= 0)
		{
			return false;
		}

		//sf::Vector2f delta;

		if (px < py) 
		{
			const float sx = sign(dx);
			//delta = { px * sx, 0.0f };
			_normal = { sx, 0.0f };
			_hitPos = { m_pos.x + (m_half.x * sx), _other.m_pos.y };
			return true;
		}
		else 
		{
			const float sy = sign(dy);
			//delta = { 0.0f, py * sy };
			_normal = { 0.0f, sy };
			_hitPos = { _other.m_pos.x, m_pos.y + (m_half.y * sy) };
			return true;
		}
	}

	void draw(sf::RenderTexture & _surface, sf::Color _color = { 255,255,255,128 });

	sf::Vector2f m_pos = { 0, 0 };
	sf::Vector2f m_half = { 0, 0 };
};