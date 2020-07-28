#include "Precomp.h"
#include "Tiles.h"
#include "Game/Game.h"
#include "Viewport/Viewport.h"

#ifndef ENABLE_INL
#include "Tiles.inl"
#endif

using namespace sf;
static const u32 g_currentVersion = 6;

//--------------------------------------------------------------------------
bool Tiles::init(String _texturePath, const sf::Vector2u & _size, const sf::Vector2u &_count, TilesType _tileType, bool _reinit)
{
	m_tilesType = _tileType;
	m_texture.loadFromFile(_texturePath);
	m_size = _size;
	m_tileInfos.resize(_count);
	m_tileInfoFilename = _texturePath + ".info"; 

	if (!_reinit)
	{
		load();
	}

	uint size = sizeof(TileInfo);

	return true;
}

//--------------------------------------------------------------------------
bool Tiles::load()
{
	FILE * fp;

	errno_t err = fopen_s(&fp, m_tileInfoFilename.toAnsiString().c_str(), "rb");

	if (err == 0)
	{
		u32 version = 0;

		uint read = 0;

		read += fread(&version, sizeof(version), 1, fp);
		read += fread(&m_size, sizeof(m_size), 1, fp);
		read += m_tileInfos.read(fp, m_tileInfoFilename.toAnsiString().c_str());

		fclose(fp);

		if (version < 3)
		{
			for (uint j = 0; j < m_tileInfos.size().y; ++j)
			{
				for (uint i = 0; i < m_tileInfos.size().x; ++i)
				{
					auto & info = m_tileInfos.get(i, j);

					info.m_flags = TileFlags::Paint;

					for (int p = 0; p < COUNT_OF(info._pad); ++p)
					{
						info._pad[p] = 0;
					}
				}
			}
		}

		if (version < 4)
		{
			for (uint j = 0; j < m_tileInfos.size().y; ++j)
			{
				for (uint i = 0; i < m_tileInfos.size().x; ++i)
				{
					auto & info = m_tileInfos.get(i, j);

					info.m_hotSpot = { 0,0 };
				}
			}
		}

		if (version < 5)
		{
			for (uint j = 0; j < m_tileInfos.size().y; ++j)
			{
				for (uint i = 0; i < m_tileInfos.size().x; ++i)
				{
					auto & info = m_tileInfos.get(i, j);

					info.m_collisionZ = info.m_box.height;
				}
			}
		}

		if (version < 6)
		{
			if (m_tilesType == TilesType::LevelTiles)
			{
				for (uint j = 0; j < m_tileInfos.size().y; ++j)
				{
					for (uint i = 0; i < m_tileInfos.size().x; ++i)
					{
						auto & info = m_tileInfos.get(i, j);

						info.m_origin = { 15,15 };
						info.m_box.left = -15;
						info.m_box.top = -15;
						info.m_box.width = 32;
						info.m_box.height = 32;
						info.m_collision.left -= 15;
						info.m_collision.top -= 15;
					}
				}
			}
		}

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Tiles::save()
{
	FILE * fp;

	errno_t err = fopen_s(&fp, m_tileInfoFilename.toAnsiString().c_str(), "wb");

	if (err == 0)
	{
		uint write = 0;

		write += fwrite(&g_currentVersion, sizeof(g_currentVersion), 1, fp);
		write += fwrite(&m_size, sizeof(m_size), 1, fp);
		write += m_tileInfos.write(fp);

		fclose(fp);
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Tiles::update()
{
	return true;
}

//--------------------------------------------------------------------------
IntRect Tiles::getTileRect(const sf::Vector2u & _coords, bool _flipX, bool _flipY) const
{
	const TileInfo & info = m_tileInfos.get(_coords);

	const int x = _coords.x * (int)m_size.x + info.m_box.left + info.m_origin.x;
	const int y = _coords.y * (int)m_size.y + info.m_box.top + info.m_origin.y;
	const int w = info.m_box.width;
	const int h = info.m_box.height;

	if (_flipX && _flipY)
		return IntRect(x+w, y+h, -w, -h);
	else if (_flipX)
		return IntRect(x+w, y, -w, h);
	else if (_flipY)
		return IntRect(x, y+h, w, -h);
	else
		return IntRect(x, y, w, h);
}

//--------------------------------------------------------------------------
bool Tiles::updateSelectTiles()
{
	Game & game = Game::get();

	Vector2f mousePos = game.m_mouse.getPosition();

	if (mousePos.x > 0 && mousePos.x < game.m_screenSize.x && mousePos.y > 0 && mousePos.y < game.m_screenSize.y)
	{
		mousePos += -m_selectTilesOffset + m_tilePresentationOffset;
		Vector2u mouseOver = Vector2u((uint)(mousePos.x / m_size.x), (uint)(mousePos.y / m_size.y));

		if (mouseOver.x < m_tileInfos.size().x && mouseOver.y < m_tileInfos.size().y)
		{
			m_mouseOverTile = mouseOver;
		}
		else
		{
			return false;
		}

		const bool ctrl = Keyboard::isKeyPressed(Keyboard::Key::LControl);

		if (game.acceptEditorInputs())
		{
			switch (game.m_editorMode)
			{
			case EditorMode::SelectDecal:
			case EditorMode::EditSprites:
			case EditorMode::SelectObject:
			case EditorMode::SelectTile:
				if (ctrl && Keyboard::isKeyPressed(Keyboard::Key::L))
				{
					load();
					game.setKeyPressed();
					return true;
				}
				else if (ctrl && Keyboard::isKeyPressed(Keyboard::Key::S))
				{
					save();
					game.setKeyPressed();
					return true;
				}
				else if (Keyboard::isKeyPressed(Keyboard::PageUp) || Mouse::isButtonPressed(Mouse::Button::XButton2))
				{
					if (m_tilePresentationOffset.y >= m_size.y)
					{
						m_tilePresentationOffset.y -= m_size.y;
					}
					game.setKeyPressed();
					return true;
				}
				else if (Keyboard::isKeyPressed(Keyboard::PageDown) || Mouse::isButtonPressed(Mouse::Button::XButton1))
				{
					if (m_tilePresentationOffset.y < game.m_screenSize.y)
					{
						m_tilePresentationOffset.y += m_size.y;
					}
					game.setKeyPressed();
					return true;
				}

				if (isValidTileSelected() && -1 != m_selectedTile.x && -1 != m_selectedTile.y)
				{
					TileInfo & info = m_tileInfos.get(m_selectedTile);

					m_drawShadow = false;
					m_drawOrigin = false;
					m_drawBox = false;
					m_drawCollision = false;
					m_drawBias = false;
					m_drawHotSpot = false;
					m_drawHeight = false;

					if (Keyboard::isKeyPressed(Keyboard::LControl))
					{
						if (Keyboard::isKeyPressed(Keyboard::C))
						{
							m_copySource = true;
							m_copyPaste = info;
						}
						else if (Keyboard::isKeyPressed(Keyboard::V))
						{
							info = m_copyPaste;
						}

						m_drawShadow = true;
						m_drawOrigin = true;
						m_drawBox = true;
						m_drawCollision = true;
						m_drawHotSpot = true;
						m_drawHeight = true;
					}

					const bool alt = Keyboard::isKeyPressed(Keyboard::Key::LAlt) || Keyboard::isKeyPressed(Keyboard::Key::RAlt);
					const bool shift = Keyboard::isKeyPressed(Keyboard::Key::LShift) || Keyboard::isKeyPressed(Keyboard::Key::RShift);

					if (alt)
					{
						m_drawFlags = true;
					}
					else
					{
						m_drawFlags = false;
					}

					if (alt && Keyboard::isKeyPressed(Keyboard::Key::P))
					{
						info.m_flags = info.m_flags ^ TileFlags::Paint;
						game.setKeyPressed();
					}
					else if (alt && Keyboard::isKeyPressed(Keyboard::Key::V))
					{
						info.m_flags = info.m_flags ^ TileFlags::Visual;
						game.setKeyPressed();
					}
					else if (alt && Keyboard::isKeyPressed(Keyboard::Key::C))
					{
						m_drawCollision = true;
						info.m_flags = info.m_flags ^ TileFlags::Collide;
						game.setKeyPressed();
					}
					else if (alt && Keyboard::isKeyPressed(Keyboard::Key::O))
					{
						info.m_flags = info.m_flags ^ TileFlags::Shadow;
						game.setKeyPressed();
					}
					
					if (Keyboard::isKeyPressed(Keyboard::Key::S))
					{
						m_drawShadow = m_drawOrigin = true;

						if (Keyboard::isKeyPressed(Keyboard::Key::Up))
						{
							info.m_shadowOffset -= 1;
							game.setKeyPressed();
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
						{
							info.m_shadowOffset += 1;
							game.setKeyPressed();
						}
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::Z) && shift)
					{
						m_drawHeight = m_drawOrigin = true;

						if (Keyboard::isKeyPressed(Keyboard::Key::Up))
						{
							info.m_collisionZ += 1;
							game.setKeyPressed();
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
						{
							info.m_collisionZ -= 1;
							game.setKeyPressed();
						}
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::O))
					{
						m_drawShadow = m_drawOrigin = true;

						if (Keyboard::isKeyPressed(Keyboard::Key::Up))
						{
							info.m_origin.y -= 1;
							game.setKeyPressed();
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
						{
							info.m_origin.y += 1;
							game.setKeyPressed();
						}

						if (Keyboard::isKeyPressed(Keyboard::Key::Left))
						{
							info.m_origin.x -= 1;
							game.setKeyPressed();
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Right))
						{
							info.m_origin.x += 1;
							game.setKeyPressed();
						}
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::H))
					{
						m_drawHotSpot = m_drawOrigin = true;

						if (Keyboard::isKeyPressed(Keyboard::Key::Up))
						{
							info.m_hotSpot.y -= 1;
							game.setKeyPressed();
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
						{
							info.m_hotSpot.y += 1;
							game.setKeyPressed();
						}

						if (Keyboard::isKeyPressed(Keyboard::Key::Left))
						{
							info.m_hotSpot.x -= 1;
							game.setKeyPressed();
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Right))
						{
							info.m_hotSpot.x += 1;
							game.setKeyPressed();
						}
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::B))
					{
						m_drawBox = true;

						if (Keyboard::isKeyPressed(Keyboard::Key::LAlt))
						{

						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::LShift))
						{
							if (Keyboard::isKeyPressed(Keyboard::Key::Up))
							{
								info.m_box.top -= 1;
								game.setKeyPressed();
							}
							else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
							{
								info.m_box.top += 1;
								game.setKeyPressed();
							}

							if (Keyboard::isKeyPressed(Keyboard::Key::Left))
							{
								info.m_box.left -= 1;
								game.setKeyPressed();
							}
							else if (Keyboard::isKeyPressed(Keyboard::Key::Right))
							{
								info.m_box.left += 1;
								game.setKeyPressed();
							}
						}
						else
						{
							if (Keyboard::isKeyPressed(Keyboard::Key::Up))
							{
								info.m_box.height -= 1;
								game.setKeyPressed();
							}
							else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
							{
								info.m_box.height += 1;
								game.setKeyPressed();
							}

							if (Keyboard::isKeyPressed(Keyboard::Key::Left))
							{
								info.m_box.width -= 1;
								game.setKeyPressed();
							}
							else if (Keyboard::isKeyPressed(Keyboard::Key::Right))
							{
								info.m_box.width += 1;
								game.setKeyPressed();
							}
						}
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::Z) && !shift)
					{
						m_drawBias = true;
						m_drawOrigin = true;

						if (Keyboard::isKeyPressed(Keyboard::Key::Up))
						{
							info.m_sortBias -= 1;
							game.setKeyPressed();
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
						{
							info.m_sortBias += 1;
							game.setKeyPressed();
						}
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::C))
					{
						m_drawCollision = true;

						if (Keyboard::isKeyPressed(Keyboard::Key::LAlt))
						{

						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::LShift))
						{
							if (Keyboard::isKeyPressed(Keyboard::Key::Up))
							{
								info.m_collision.top -= 1;
								game.setKeyPressed();
							}
							else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
							{
								info.m_collision.top += 1;
								game.setKeyPressed();
							}

							if (Keyboard::isKeyPressed(Keyboard::Key::Left))
							{
								info.m_collision.left -= 1;
								game.setKeyPressed();
							}
							else if (Keyboard::isKeyPressed(Keyboard::Key::Right))
							{
								info.m_collision.left += 1;
								game.setKeyPressed();
							}
						}
						else
						{
							if (Keyboard::isKeyPressed(Keyboard::Key::Up))
							{
								info.m_collision.height -= 1;
								game.setKeyPressed();
							}
							else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
							{
								info.m_collision.height += 1;
								game.setKeyPressed();
							}

							if (Keyboard::isKeyPressed(Keyboard::Key::Left))
							{
								info.m_collision.width -= 1;
								game.setKeyPressed();
							}
							else if (Keyboard::isKeyPressed(Keyboard::Key::Right))
							{
								info.m_collision.width += 1;
								game.setKeyPressed();
							}
						}
					}
					else
					{
						if (Keyboard::isKeyPressed(Keyboard::Key::Up))
						{
							m_selectedTile.y = ((int)(m_selectedTile.y) - 1) % m_tileInfos.size().y;
							game.setKeyPressed();
							updateTilePresentationOffset();
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
						{
							m_selectedTile.y = (m_selectedTile.y + 1) % m_tileInfos.size().y;
							game.setKeyPressed();
							updateTilePresentationOffset();
						}

						if (Keyboard::isKeyPressed(Keyboard::Key::Right))
						{
							m_selectedTile.x = (m_selectedTile.x + 1) % m_tileInfos.size().x;
							game.setKeyPressed();
							updateTilePresentationOffset();
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Left))
						{
							if (m_selectedTile.x == 0)
								m_selectedTile.x = m_tileInfos.size().x - 1;
							else
								m_selectedTile.x = m_selectedTile.x - 1;
							game.setKeyPressed();
							updateTilePresentationOffset();
						}
					}
				}
				break;
			}

			if (game.isMouseButtonJustPressed(Mouse::Button::Left))
			{
				if (m_selectedTile != m_mouseOverTile)
				{
					m_selectedTile = m_mouseOverTile;
					updateTilePresentationOffset();
					game.setMouseButtonJustPressed(Mouse::Button::Left, false);
					return true;
				}
				else
				{
					const auto & info = m_tileInfos.get(m_selectedTile);
					if (asBool(info.m_flags & TileFlags::Paint))
					{
						switch (game.m_editorMode)
						{
							case EditorMode::SelectTile:
								game.m_editorMode = EditorMode::PaintTile;
								break;

							case EditorMode::SelectDecal:
								game.m_editorMode = EditorMode::PaintDecal;
								break;

							case EditorMode::SelectObject:
								game.m_editorMode = EditorMode::PaintObject;
								break;

							case EditorMode::EditSprites:
								break;

							default:
								assert(false);
								break;
						}

						game.m_level.m_canPaint = false;
						game.setMouseButtonJustPressed(Mouse::Button::Left, false);
						return true;
					}
				}
			}
		}
	}

	return false;
}

//--------------------------------------------------------------------------
void Tiles::updateTilePresentationOffset()
{
	if (isValidTileSelected() && m_selectedTile.x != -1 && m_selectedTile.y != -1)
	{
		const Game & game = Game::get();

		int maxTileX = (game.m_screenSize.x - m_selectTilesOffset.x) / m_size.x;
		int maxTileY = (game.m_screenSize.y - m_selectTilesOffset.y) / m_size.y;

		int curTileX = m_selectedTile.x;
		int curTileY = m_selectedTile.y;

		int offsetX = m_tilePresentationOffset.x / m_size.x;
		int offsetY = m_tilePresentationOffset.y / m_size.y;

		if (curTileX - offsetX >= maxTileX)
			offsetX = max(offsetX, curTileX - maxTileX + 1);

		if (curTileX - offsetX < 0)
			offsetX = curTileX;

		if (curTileY - offsetY >= maxTileY)
			offsetY = max(offsetY, curTileY - maxTileY + 1);

		if (curTileY - offsetY < 0)
			offsetY = curTileY;

		m_tilePresentationOffset.x = offsetX * m_size.x;
		m_tilePresentationOffset.y = offsetY * m_size.y;
	}
}

//--------------------------------------------------------------------------
sf::Color Tiles::getColor() const
{
	switch (m_tilesType)
	{
		case TilesType::LevelTiles:
			return Color(128, 128, 255, 255);

		case TilesType::DecalTiles:
			return Color(255, 128, 128, 255);

		case TilesType::ObjectTiles:
			return  Color(128, 255, 128, 255);

		case TilesType::SpriteTiles:
			return Color(128, 128, 128, 255);
	}

	return Color(0, 0, 0, 0);
}

//--------------------------------------------------------------------------
sf::Color Tiles::getTransparentColor() const
{
	Color color = getColor();
	return Color(color.r, color.g, color.b, color.a / 2);
}

//--------------------------------------------------------------------------
sf::Color Tiles::getBackgroundColor() const
{
	switch (m_tilesType)
	{
		default:
		case TilesType::LevelTiles:
		case TilesType::ObjectTiles:
		case TilesType::DecalTiles:
		case TilesType::SpriteTiles:
			return Color(255, 0, 255, 255);
	}
}

//--------------------------------------------------------------------------
bool Tiles::isValidTileSelected() const
{
	return m_mouseOverTile.x >= 0 && m_mouseOverTile.x < count().x && m_mouseOverTile.y >= 0 && m_mouseOverTile.y < count().y;
}

//--------------------------------------------------------------------------
bool Tiles::drawSelectTiles(sf::RenderTexture & _surface)
{
	Game & game = Game::get();

	_surface.clear(getBackgroundColor());

	const Vector2f offset = m_selectTilesOffset - m_tilePresentationOffset;

	// First pass to draw images
	for (uint y = 0; y < count().y; ++y)
	{
		for (uint x = 0; x < count().x; ++x)
		{
			const TileInfo & info = m_tileInfos.get({ x,y });

			Sprite tile;
				   tile.setTexture(m_texture);
				   tile.setTextureRect(IntRect(x * m_size.x, y * m_size.y, m_size.x, m_size.y));
				   tile.setPosition((float)(x * m_size.x) + offset.x + (float)info.m_origin.x, (float)(y * m_size.y) + offset.y + (float)info.m_origin.y);
				   tile.setOrigin(Vector2f(info.m_origin));

			const bool canPaint = asBool(info.m_flags & TileFlags::Paint);
			const bool canCollide = asBool(info.m_flags & TileFlags::Collide);
			const bool canSpawnVisual = asBool(info.m_flags & TileFlags::Visual);

			if (!canPaint)
			{
				RectangleShape bg;
							   bg.setPosition(Vector2f(x*m_size.x + offset.x, y*m_size.y + offset.y));
							   bg.setOrigin(Vector2f(0, 0));
							   bg.setSize(Vector2f(32,32));
							   bg.setFillColor({ 0,0,0,128 });
				_surface.draw(bg);
			}

			const bool selectedTile = x == m_selectedTile.x && y == m_selectedTile.y;

			if (selectedTile)
			{
				if (m_drawShadow)
				{
					// draw shadow
					Sprite shadow;
						   shadow.setTextureRect(getTileRect({ x,y }));
						   shadow.setOrigin(-Vector2f(info.m_box.left, info.m_box.top));
						   shadow.setTexture(m_texture);
						   shadow.setPosition((float)(x * m_size.x) + offset.x + (float)info.m_origin.x, (float)(y * m_size.y) + offset.y + (float)info.m_origin.y);
						   shadow.setOrigin(shadow.getOrigin().x, 0);
						   shadow.setScale({ 1, -0.5f });
						   shadow.setColor({ 0,0,0,64 });
						   shadow.move(Vector2f(0, info.m_shadowOffset + m_size.y / 2));

					_surface.draw(shadow);
				}
			}

			_surface.draw(tile);
		}
	}

	// Second pass to draw boxes
	for (uint y = 0; y < count().y; ++y)
	{
		for (uint x = 0; x < count().x; ++x)
		{
			const TileInfo & info = m_tileInfos.get({ x,y });

			Sprite tile;
				   tile.setTexture(m_texture);
				   tile.setTextureRect(IntRect(x * m_size.x, y * m_size.y, m_size.x, m_size.y));
				   tile.setPosition((float)(x * m_size.x) + offset.x + (float)info.m_origin.x, (float)(y * m_size.y) + offset.y + (float)info.m_origin.y);
				   tile.setOrigin(Vector2f(info.m_origin));

			const bool canPaint = asBool(info.m_flags & TileFlags::Paint);
			const bool canCollide = asBool(info.m_flags & TileFlags::Collide);
			const bool canSpawnVisual = asBool(info.m_flags & TileFlags::Visual);

			const bool selectedTile = x == m_selectedTile.x && y == m_selectedTile.y;

			if (selectedTile)
			{
				if (m_drawBox)
				{
					RectangleShape box;
					box.setPosition(tile.getPosition());
					box.setSize(Vector2f(info.m_box.width - 2, info.m_box.height - 2));
					box.setFillColor(Color(0, 0, 0, 0));
					box.setOutlineColor(Color(255, 255, 255, 128));
					box.setOutlineThickness(1);
					box.move(Vector2f(info.m_box.left + 1, info.m_box.top + 1));

					_surface.draw(box);
				}

				if (m_drawCollision)
				{
					RectangleShape box;
					box.setPosition(tile.getPosition());
					box.setSize(Vector2f(info.m_collision.width - 2, info.m_collision.height - 2));
					box.setFillColor(Color(0, 0, 0, 0));
					box.setOutlineColor(Color(255, 255, 0, 128));
					box.setOutlineThickness(1);
					box.move(Vector2f(info.m_collision.left + 1, info.m_collision.top + 1));

					_surface.draw(box);
				}

				if (m_drawOrigin)
				{
					RectangleShape origin;
								   origin.setPosition(tile.getPosition() - Vector2f(info.m_origin));
								   origin.setSize({ 1, 1 });
								   origin.setFillColor(Color(128, 255, 128, 128));
								   origin.move(Vector2f(info.m_origin));

					_surface.draw(origin);
				}

				if (m_drawHotSpot)
				{
					RectangleShape hotSpot;
								   hotSpot.setPosition(tile.getPosition() - Vector2f(info.m_origin) + Vector2f(info.m_hotSpot));
								   hotSpot.setSize({ 1, 1 });
								   hotSpot.setFillColor(Color(128, 128, 255, 128));
								   hotSpot.move(Vector2f(info.m_origin));

					_surface.draw(hotSpot);
				}

				if (m_drawHeight)
				{
					RectangleShape height;
								   height.setPosition(tile.getPosition() - Vector2f(info.m_origin));
								   height.setSize({ 1, -(float)info.m_collisionZ });
								   height.setFillColor(Color(255, 255, 255, 128));
								   height.move(Vector2f(info.m_origin));

					_surface.draw(height);
				}

				if (m_drawBias)
				{
					RectangleShape origin;
								   origin.setPosition(tile.getPosition() - Vector2f(info.m_origin));
								   origin.setSize({ 1, 1 });
								   origin.setFillColor(Color(255, 128, 128, 128));
								   origin.move(Vector2f(info.m_origin.x, info.m_origin.y + info.m_sortBias));

					_surface.draw(origin);
				}
			}

			if (m_drawFlags)
			{
				if (!canPaint || !canCollide || !canSpawnVisual)
				{
					RectangleShape bg;
					bg.setPosition(Vector2f(x*m_size.x + offset.x, y*m_size.y + offset.y));
					bg.setOrigin(Vector2f(0, 0));
					bg.setSize(Vector2f(32, 32));
					bg.setFillColor(Color(canPaint ? 255 : 0, canCollide ? 255 : 0, canSpawnVisual ? 255 : 0, 128));

					_surface.draw(bg);
				}
			}

			RectangleShape rect;
						   rect.setPosition((float)(x * m_size.x + 1 + offset.x), (float)(y * m_size.y + 1 + offset.y));
						   rect.setSize(Vector2f((float)(m_size.x - 2), (float)(m_size.y - 2)));
						   rect.setFillColor(Color(0, 0, 0, 0));
						   rect.setOutlineThickness(1.0f);
						   rect.setOutlineColor(Color(0, 0, 0, 32));

			_surface.draw(rect);
		}
	}

	RectangleShape rect;
	rect.setSize(Vector2f(game.m_screenSize.x, m_selectTilesOffset.y));
	rect.setFillColor(getColor());

	_surface.draw(rect);

	if (isValidTileSelected())
	{
		Color selectedColor = { 255,255,255,128 }; 
		Color mouseOverColor = getTransparentColor();

		const uint index = getSelectedTileIndex();

		if ((uint)-1 != index)
		{
			RectangleShape rect;
						   rect.setPosition((float)(m_selectedTile.x * m_size.x + 1 + offset.x), (float)(m_selectedTile.y * m_size.y + 1 + offset.y));
						   rect.setSize(Vector2f((float)(m_size.x - 2), (float)(m_size.y - 2)));
						   rect.setFillColor(Color(0, 0, 0, 0));
						   rect.setOutlineColor(selectedColor);
						   rect.setOutlineThickness(1.0f);

			_surface.draw(rect);

			const TileInfo & info = m_tileInfos.get(m_selectedTile);

			char temp[512];

			sprintf_s(temp, "Coords %u %u\n"     
							"Index %3u    [o]rigine:%4i,%4i  Boite:        Collision:   [alt-p]eindre  :%s\n"
							"             [h]otSpot:%4i,%4i  [B]%4i,%4i  [C]%4i,%4i [alt-v]isuel   :%s\n"
							"             [s]hadow :%4i       [b]%4i,%4i  [c]%4i,%4i [alt-c]ollision:%s\n"
							"             [z]bias  :%4i       [Z]col  %4i",
				m_selectedTile.x, m_selectedTile.y,
				coordsToIndex(m_selectedTile),
				info.m_origin.x, info.m_origin.y, asBool(info.m_flags & TileFlags::Paint) ? "OUI" : "NON", 
				info.m_hotSpot.x, info.m_hotSpot.y, info.m_box.left, info.m_box.top, info.m_collision.left, info.m_collision.top, asBool(info.m_flags & TileFlags::Visual) ? "OUI" : "NON",
				info.m_shadowOffset, info.m_box.width, info.m_box.height, info.m_collision.width, info.m_collision.height, asBool(info.m_flags & TileFlags::Collide) ? "OUI" : "NON",
				info.m_sortBias, info.m_collisionZ);

			if (m_tilesType == TilesType::DecalTiles)
			{
				sprintf_s(temp, "%s               [alt-o]mbre    :%s\n", temp, asBool(info.m_flags & TileFlags::Shadow) ? "OUI" : "NON");
			}

			game.m_debugString += String(temp);
		}
	}
	
	return true;
}

//--------------------------------------------------------------------------
uint Tiles::getSelectedTileIndex() const
{
	if (m_selectedTile.x >= 0 && m_selectedTile.x < count().x && m_selectedTile.y >= 0 && m_selectedTile.y < count().y)
	{
		return m_selectedTile.y * count().x + m_selectedTile.x;
	}
	else
	{
		return (uint)-1;
	}
}

//--------------------------------------------------------------------------
const char * Tiles::getTileTypeName() const
{
	switch (m_tilesType)
	{
		case TilesType::LevelTiles:
			return "LevelTiles";

		case TilesType::ObjectTiles:
			return "ObjectTiles";

		case TilesType::DecalTiles:
			return "DecalTiles";

		case TilesType::SpriteTiles:
			return "SpriteTiles";

		default:
			return nullptr;
	}
}
//--------------------------------------------------------------------------
CellDataIndex Tiles::getCellInfoIndex() const
{
	switch (m_tilesType)
	{
		case TilesType::LevelTiles:
			return CellDataIndex::Tile;

		case TilesType::ObjectTiles:
			return CellDataIndex::Object;

		case TilesType::DecalTiles:
			return CellDataIndex::Decal;

		case TilesType::SpriteTiles:
		default:
			return (CellDataIndex)-1;
	}
}
