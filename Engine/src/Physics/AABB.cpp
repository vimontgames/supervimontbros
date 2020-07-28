#include "Precomp.h"
#include "AABB.h"

using namespace sf;

void AABB::draw(sf::RenderTexture & _surface, sf::Color _color)
{
	if (m_half.x * m_half.y > 0.0f)
	{
		sf::RectangleShape box;
						   box.setPosition(m_pos - m_half + Vector2f(1,1));
						   box.setSize(m_half * 2.0f - Vector2f(2,2));
						   box.setFillColor(Color(0, 0, 0, 0));
						   box.setOutlineColor(_color);
						   box.setOutlineThickness(1);

		_surface.draw(box);
	}
}