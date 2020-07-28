#include "Precomp.h"
#include "Level.h"
#include "Game/Game.h"
#include "Physics/AABB.h"
#include "Viewport/Viewport.h"
#include "Tiles/Tiles.h"
#include "Shader/Shader.h"

#ifndef ENABLE_INL
#include "Level.inl"
#endif

using namespace sf;

// Version 1 : Save background and object tilemap sizes
// Version 2 : Background and object tiles using 16-bits indices
// Version 3 : Add decals
static const u32 g_currentVersion = 3;

//--------------------------------------------------------------------------
bool Level::init(sf::String _filename, const sf::Vector2u & _count, const Tiles * _backgroundTiles, const Tiles * _decalTiles, const Tiles * _objectTiles)
{
	m_filename = _filename;
	resize(_count);

	m_backgroundTiles = _backgroundTiles;
	m_decalTiles = _decalTiles;
	m_objectTiles = _objectTiles;

	load();

	m_paintCamera = Game::get().getDefaultViewport().getCameraOffset();

	return true;
}

//--------------------------------------------------------------------------
bool Level::resize(const sf::Vector2u & _count)
{
	m_cells.resize(_count);
	return true;
}

//--------------------------------------------------------------------------
bool Level::load()
{
	FILE * fp;

	errno_t err = fopen_s(&fp, m_filename.toAnsiString().c_str(), "rb");

	if (err == 0)
	{
		uint read = 0;

		u32 version;
		read += (uint)fread(&version, sizeof(version), 1, fp);

		Vector2u savedBackgroundTileDim = m_backgroundTiles->count();
		read += (uint)fread(&savedBackgroundTileDim, sizeof(savedBackgroundTileDim), 1, fp);

		Vector2u saveDecalTileDim = Vector2u(0,0);
		if (version >= 3)
		{
			saveDecalTileDim = m_decalTiles->count();
			read += (uint)fread(&saveDecalTileDim, sizeof(saveDecalTileDim), 1, fp);
		}

		Vector2u saveObjectTileDim = m_objectTiles->count();
		read += (uint)fread(&saveObjectTileDim, sizeof(saveObjectTileDim), 1, fp);

		if (version < 2)
		{
			// Read old format to convert 8-bits to 16-bits indices
			struct CellInfoV1
			{
				u8 tile;
				u8 obj;
			};

			Array2D<CellInfoV1> cellsV1;
			cellsV1.resize(m_cells.size());
			read += cellsV1.read(fp, m_filename.toAnsiString().c_str());

			assert(m_cells.size() == cellsV1.size());

			for (u32 j = 0; j < m_cells.size().y; ++j)
			{
				for (u32 i = 0; i < m_cells.size().x; ++i)
				{
					const auto cellV1 = cellsV1.get(i, j);
					auto & cell = m_cells.get(i, j);

					cell.tile.index = cellV1.tile;
					cell.tile.offset = { 0,0 };
					cell.tile.palette = 0;
					cell.tile.flags = CellFlags::None;

					cell.decal.index = 0;
					cell.decal.offset = { 0,0 };
					cell.decal.palette = 0;
					cell.decal.flags = CellFlags::None;

					cell.obj.index = cellV1.obj;
					cell.obj.offset = { 0,0 };
					cell.obj.palette = 0;
					cell.obj.flags = CellFlags::None;

					assert(cell.tile.index < 256);
					assert(cell.obj.index < 256);
				}
			}
		}
		else if (version == 2) // 2 to 3
		{
			struct CellInfoV2
			{
				tileIndex tile;
				objectIndex obj;
				Vector2i8 offset;
				u8		  palette;
				CellFlags flags;
			};

			Array2D<CellInfoV2> cellsV2;
			cellsV2.resize(m_cells.size());
			read += cellsV2.read(fp, m_filename.toAnsiString().c_str());

			assert(m_cells.size() == cellsV2.size());

			for (u32 j = 0; j < m_cells.size().y; ++j)
			{
				for (u32 i = 0; i < m_cells.size().x; ++i)
				{
					const auto cellV2 = cellsV2.get(i, j);
					auto & cell = m_cells.get(i, j);

					cell.tile.index = cellV2.tile;
					cell.tile.offset = { 0,0 };
					cell.tile.palette = 0;
					cell.tile.flags = CellFlags::None;

					cell.decal.index = 0;
					cell.decal.offset = { 0,0 };
					cell.decal.palette = 0;
					cell.decal.flags = CellFlags::None;

					cell.obj.index = cellV2.obj;
					cell.obj.offset = cellV2.offset;
					cell.obj.palette = cellV2.palette;
					cell.obj.flags = cellV2.flags;

					assert(cell.tile.index < 256);
					assert(cell.decal.index < 256);
					assert(cell.obj.index < 256);
				}
			}
		}
		else
		{ 
			read += m_cells.read(fp, m_filename.toAnsiString().c_str());
		}

		onTileSetResized(m_backgroundTiles, savedBackgroundTileDim);
		onTileSetResized(m_decalTiles, saveDecalTileDim);
		onTileSetResized(m_objectTiles, saveObjectTileDim);

		// clean
		for (u32 j = 0; j < m_cells.size().y; ++j)
		{
			for (u32 i = 0; i < m_cells.size().x; ++i)
			{
				auto & cell = m_cells.get(i, j);

				const auto tileCoords = m_backgroundTiles->indexToCoords(cell.tile.index);
				const auto & tileInfo = m_backgroundTiles->m_tileInfos.get(tileCoords);

				if (!asBool(tileInfo.m_flags & TileFlags::Paint))
					debugPrint("Tile {%u, %u} at {%u, %u} is not paintable\n", tileCoords.x, tileCoords.y, i, j);

				const auto objCoords = m_objectTiles->indexToCoords(cell.obj.index);
				const auto & objInfo = m_objectTiles->m_tileInfos.get(objCoords);

				if (!asBool(objInfo.m_flags & TileFlags::Paint))
					debugPrint("Object {%u, %u} at {%u, %u} is not paintable\n", objCoords.x, objCoords.y, i, j);
			}
		}

		fclose(fp);
		m_tilesVertexArrayDirty = true;
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Level::onTileSetResized(const Tiles * _tileSet, const sf::Vector2u _previousSize)
{
	if (_tileSet && _tileSet->count() != _previousSize)
	{
		debugPrint("%s dimensions have changed from {%u,%u} to {%u,%u}\n", _tileSet->getTileTypeName(), _previousSize.x, _previousSize.y, _tileSet->count().x, _tileSet->count().y);

		const auto index = _tileSet->getCellInfoIndex();
		
		for (u32 j = 0; j < m_cells.size().y; ++j)
		{
			for (u32 i = 0; i < m_cells.size().x; ++i)
			{
				auto & cell = m_cells.get(i, j);
				auto & data = cell.data[(uint)index];

				const auto & coords = _tileSet->indexToCoords(data.index, _previousSize.x);
				const uint newTileIndex = _tileSet->coordsToIndex(coords);

				if (newTileIndex != data.index)
				{
					debugPrint("cell.data[%u] changed from {%u} to {%u}\n", (uint)index, data.index, newTileIndex);
					data.index = newTileIndex;
				}

			}
		}

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Level::save()
{
	FILE * fp;

	errno_t err = fopen_s(&fp, m_filename.toAnsiString().c_str(), "wb");

	if (err == 0)
	{
		uint write = 0;

		write += (uint)fwrite(&g_currentVersion, sizeof(g_currentVersion), 1, fp);
		
		const auto bgTileDim = m_backgroundTiles->count();
		write += (uint)fwrite(&bgTileDim, sizeof(bgTileDim), 1, fp);

		const auto decalTileDim = m_decalTiles->count();
		write += (uint)fwrite(&decalTileDim, sizeof(decalTileDim), 1, fp);
		
		const auto objTileDim = m_objectTiles->count();
		write += (uint)fwrite(&objTileDim, sizeof(objTileDim), 1, fp);

		write += m_cells.write(fp);		

		fclose(fp);
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
const sf::Vector2u Level::getTilePos(const sf::Vector2f & _position) const
{
	Game & game = Game::get();
	Vector2u coords = Vector2u(_position.x / game.m_levelTiles.m_size.x, _position.y / game.m_levelTiles.m_size.y);
	return Vector2u(clamp(coords.x, 0U, m_cells.size().x), clamp(coords.y, 0U, m_cells.size().y));
}

//--------------------------------------------------------------------------
const CellInfo & Level::getTileUnderPos(const sf::Vector2f & _position) const
{
	return m_cells.get(getTilePos(_position));
}

//--------------------------------------------------------------------------
bool Level::updatePainting()
{
	Game & game = Game::get();
	const float dt = game.m_dt;

	if (game.isMouseOverScreen())
	{
		const Vector2f mousePos = game.m_mouse.getPosition() + (m_paintCamera)-Vector2f(game.m_screenSize.x / 2.0f, game.m_screenSize.y / 2.0f);

		m_mouseOverTile = Vector2u((uint)(mousePos.x / game.m_levelTiles.m_size.x), (uint)(mousePos.y / game.m_levelTiles.m_size.y));

		if (m_canPaint == false)
		{
			static bool neverPressed = true; // hack for the 1st clic

			if (game.isMouseButtonJustReleased(Mouse::Button::Left) || neverPressed)
			{
				m_canPaint = true;
				neverPressed = false;
			}
		}

		if ((EditorMode::PaintTile == game.m_editorMode || EditorMode::PaintObject == game.m_editorMode || EditorMode::PaintDecal == game.m_editorMode) && m_canPaint)
		{
			if (Keyboard::isKeyPressed(Keyboard::LControl))
			{
				if (Keyboard::isKeyPressed(Keyboard::Key::A))
				{
					game.setSelectedRect({ 0,0 }, { (int)m_cells.size().x - 1, (int)(m_cells.size().y - 1) });
					game.setKeyPressed();
					return true;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Key::X))
				{
					Vector2i m, M;
					if (game.getSelectedRect(m, M))
					{
						copySelectedRect(m, M, true);
					}
					game.resetSelectedRect();
					game.setKeyPressed();
					return true;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Key::C))
				{
					Vector2i m, M;
					if (game.getSelectedRect(m, M))
					{
						copySelectedRect(m, M, false);
					}
					game.resetSelectedRect();
					game.setKeyPressed();
				}
				else if (Keyboard::isKeyPressed(Keyboard::Key::V))
				{
					const auto & copy = m_selectedCopy;
					if (copy.size().x > 0 && copy.size().y > 0)
					{
						switch (game.m_editorMode)
						{
						case EditorMode::PaintTile:
						case EditorMode::PaintObject:
						case EditorMode::PaintDecal:
							for (u32 y = 0; y < copy.size().y; ++y)
							{
								for (u32 x = 0; x < copy.size().x; ++x)
								{
									const auto & cell = copy.get({ x,y });
									const Vector2u pos = { m_mouseOverTile.x + x, m_mouseOverTile.y + y };

									if (pos.x < m_cells.size().x && pos.y < m_cells.size().y)
									{
										m_cells.set(pos, cell);
									}
								}
							}
							break;
						}
					}

					return true;
				}
			}
			else
			{
				if (game.canPressKey())
				{
					auto & cell = m_cells.get(m_mouseOverTile).data[(uint)getCurrentCellInfoIndex()];
	
					if (Keyboard::isKeyPressed(Keyboard::Key::C))
					{
						game.m_drawCollisions = !game.m_drawCollisions;
						game.setKeyPressed();
						return true;
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::P))
					{
						cell.palette = (cell.palette + 1) % game.m_spriteLutCount;
						game.setKeyPressed();
						return true;
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::X))
					{
						cell.flags ^= CellFlags::FlipX;
						game.setKeyPressed();
						return true;
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::Y))
					{
						cell.flags ^= CellFlags::FlipY;
						game.setKeyPressed();
						return true;
					}
					if (Keyboard::isKeyPressed(Keyboard::Key::O))
					{
						if (Keyboard::isKeyPressed(Keyboard::Key::Left))
						{
							--cell.offset.x;
							game.setKeyPressed();
							return true;
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Right))
						{
							++cell.offset.x;
							game.setKeyPressed();
							return true;
						}

						if (Keyboard::isKeyPressed(Keyboard::Key::Up))
						{
							--cell.offset.y;
							game.setKeyPressed();
							return true;
						}
						else if (Keyboard::isKeyPressed(Keyboard::Key::Down))
						{
							++cell.offset.y;
							game.setKeyPressed();
							return true;
						}
					}

					if (game.isMouseButtonPressed(Mouse::Button::Left))
					{
						switch (game.m_editorMode)
						{
							case EditorMode::PaintTile:
								setTile(m_mouseOverTile, game.m_levelTiles.getSelectedTileIndex());
								break;

							case EditorMode::PaintDecal:
								setDecal(m_mouseOverTile, game.m_decalTiles.getSelectedTileIndex());
								break;

							case EditorMode::PaintObject:
								setObject(m_mouseOverTile, game.m_objectTiles.getSelectedTileIndex());
								break;
						}

						game.setMouseButtonJustPressed(Mouse::Button::Left, false);
						return true;
					}
					else if (Keyboard::isKeyPressed(Keyboard::Key::Delete))
					{
						switch (game.m_editorMode)
						{
							case EditorMode::PaintTile:
								setTile(m_mouseOverTile, 0);
								break;

							case EditorMode::PaintDecal:
								setDecal(m_mouseOverTile, 0);
								break;

							case EditorMode::PaintObject:
								setObject(m_mouseOverTile, 0);
								break;
						}

						//game.setKeyPressed();
						return true;
					}
				}
			}

			const float speed = 1.0f;

			if (!Keyboard::isKeyPressed(Keyboard::Key::O))
			{
				if (Keyboard::isKeyPressed(Keyboard::Key::Right))
				{
					m_paintCamera.x += dt * speed;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Key::Left))
				{
					m_paintCamera.x -= dt * speed;
				}

				if (Keyboard::isKeyPressed(Keyboard::Key::Down))
				{
					m_paintCamera.y += dt * speed;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Key::Up))
				{
					m_paintCamera.y -= dt * speed;
				}
			}

			m_paintCamera.x = max(m_paintCamera.x, (game.m_screenSize.x / 2.0f));
			m_paintCamera.x = min(m_paintCamera.x, (game.m_level.count().x * game.m_levelTiles.m_size.x - game.m_screenSize.x / 2.0f));

			m_paintCamera.y = max(m_paintCamera.y, (game.m_screenSize.y / 2.0f));
			m_paintCamera.y = min(m_paintCamera.y, (game.m_level.count().y * game.m_levelTiles.m_size.y - game.m_screenSize.y / 2.0f));
		}
	}

	return false;
}

//--------------------------------------------------------------------------
bool Level::update()
{
	const bool needUpdate = updatePainting();

	if (needUpdate)
	{
		m_tilesVertexArrayDirty = true;
	}

	if (m_tilesVertexArrayDirty)
	{
		buildTilesVertexArray();
		m_tilesVertexArrayDirty = false;
	}

	return needUpdate;
}

//--------------------------------------------------------------------------
void Level::copySelectedRect(const Vector2i & _min, const Vector2i & _max, bool _cut)
{
	Game & game = Game::get();

	const auto w = (u32)(_max.x - _min.x + 1);
	const auto h = (u32)(_max.y - _min.y + 1);

	m_selectedCopy.resize({ w, h });

	for (u32 y = 0; y < h; y++)
	{
		for (u32 x = 0; x < w; x++)
		{
			const Vector2u srcCoords = { _min.x + x, _min.y + y };
			m_selectedCopy.set({ x, y }, get(srcCoords));
		}
	}

	if (_cut)
	{
		for (u32 y = 0; y < h; y++)
		{
			for (u32 x = 0; x < w; x++)
			{
				const Vector2u srcCoords = { _min.x + x, _min.y + y };

				m_cells.set(srcCoords, CellInfo());
			}
		}
	}
}

//--------------------------------------------------------------------------
CellDataIndex Level::getCurrentCellInfoIndex() const
{
	Game & game = Game::get();
	switch (game.m_editorMode)
	{
		case EditorMode::SelectTile:
		case EditorMode::PaintTile:
			return CellDataIndex::Tile;

		case EditorMode::SelectDecal:
		case EditorMode::PaintDecal:
			return CellDataIndex::Decal;

		case EditorMode::SelectObject:
		case EditorMode::PaintObject:
			return CellDataIndex::Object;

		default:
			return (CellDataIndex) - 1;
	}
}

//--------------------------------------------------------------------------
const Tiles * Level::getCurrentTileSet() const
{
	Game & game = Game::get();
	switch (game.m_editorMode)
	{
		case EditorMode::SelectTile:
		case EditorMode::PaintTile:
			return m_backgroundTiles;

		case EditorMode::SelectDecal:
		case EditorMode::PaintDecal:
			return m_decalTiles;

		case EditorMode::SelectObject:
		case EditorMode::PaintObject:
			return m_objectTiles;

		default:
			return nullptr;
	}
}

//--------------------------------------------------------------------------
bool Level::drawPainting(sf::RenderTexture & _surface)
{
	Game & game = Game::get();

	if (game.m_editorMode == EditorMode::PaintTile || game.m_editorMode == EditorMode::PaintObject || game.m_editorMode == EditorMode::PaintDecal)
	{
		Game & game = Game::get();

		if (!(m_mouseOverTile.x < m_cells.size().x && m_mouseOverTile.y < m_cells.size().y))
			return false;

		const auto mouseOverInfo = get(m_mouseOverTile);

		const Tiles * tileSet = getCurrentTileSet();
		const CellIndex<u16> & info = mouseOverInfo.data[(uint)getCurrentCellInfoIndex()];
		
		Vector2f camOffset;
		         camOffset.x = game.m_level.m_paintCamera.x - game.m_screenSize.x / 2.0f;
		         camOffset.y = game.m_level.m_paintCamera.y - game.m_screenSize.y / 2.0f;

		Vector2u camTile;
		         camTile.x = (uint)(camOffset.x / (float)tileSet->m_size.x);
		         camTile.y = (uint)(camOffset.y / (float)tileSet->m_size.y);

		const auto selIndex = tileSet->getSelectedTileIndex();
		const auto selCoords = tileSet->indexToCoords(selIndex);
		const auto & selInfo = get(selCoords);		
		
		char temp[512];
		sprintf_s(temp, "Clic-gauche souris pour peindre avec brique %u (%u, %u) en  (%u, %u = %u)\n"
			"Clic-droit souris pour choisir une autre brique ou changer de mode.\n"
			"[p] Palette: %u [x] Flip:%s [y] Flip:%s [o]ffset: %i %i [Ctrl-S]auvegarder \n",
			selIndex, selCoords.x, selCoords.y,
			game.m_level.m_mouseOverTile.x, game.m_level.m_mouseOverTile.y, info.index,
			info.palette, asBool(info.flags & CellFlags::FlipX) ? "oui" : "non", asBool(info.flags & CellFlags::FlipY) ? "oui" : "non",
			info.offset.x, info.offset.y);

		game.m_debugString += String(temp);

		Vector2i m, M;
		if (Keyboard::isKeyPressed(Keyboard::LControl) && game.getSelectedRect(m, M))
		{
			sprintf_s(temp, "Sélection de (%i, %i) à (%i, %i)", m.x, m.y, M.x, M.y);
			game.m_debugString += String(temp);
		}

		const Vector2u mousePos = Vector2u((uint)game.m_mouse.getPosition().x, (uint)game.m_mouse.getPosition().y) + Vector2u(m_paintCamera) - Vector2u(game.m_screenSize.x / 2, game.m_screenSize.y / 2);

		const Vector2u curCell = m_mouseOverTile;	
		const Vector2u selectedTile = tileSet->m_selectedTile;
		const Color borderColor = Color(255, 128, 128, 128);
		
		IntRect brushRect;
		Vector2f brushPos;
		Vector2i origin = { 0,0 };

		bool draw = false;		

		if (tileSet->isValidTileSelected() && m_selectedTile.x != -1 && m_selectedTile.y != -1)
		{
			const auto & tileInfo = tileSet->m_tileInfos.get(selectedTile);

			origin = tileInfo.m_origin;

			brushRect = IntRect(selectedTile.x * tileSet->m_size.x + tileInfo.m_origin.x + tileInfo.m_box.left,
				selectedTile.y * tileSet->m_size.y + tileInfo.m_origin.y + tileInfo.m_box.top,
				tileInfo.m_box.width,
				tileInfo.m_box.height);

			brushPos = Vector2f((float)(curCell.x * tileSet->m_size.x + 15 + info.offset.x), (float)(curCell.y * tileSet->m_size.y + 15 + info.offset.y));

			draw = true;
		}

		const auto & selectionCopy = m_selectedCopy;

		if (Keyboard::isKeyPressed(Keyboard::LControl))
		{
			Vector2i m, M;

			if (game.getSelectedRect(m, M))
			{
				RectangleShape rect;
				rect.setOutlineColor(borderColor);
				rect.setPosition(Vector2f((float)(m.x * game.m_levelTiles.m_size.x + 2), (float)(m.y * game.m_levelTiles.m_size.y + 2)));
				rect.setSize(Vector2f((float)((M.x - m.x + 1) * game.m_levelTiles.m_size.x - 4), (float)((M.y - m.y + 1) * game.m_levelTiles.m_size.y - 4)));
				rect.setOutlineColor(Color(255, 255, 255, 128));
				rect.setFillColor(Color(255, 255, 255, 64));
				rect.setOutlineThickness(2.0f);

				_surface.draw(rect);
			}

			if (selectionCopy.size().x > 0 && selectionCopy.size().y > 0 && !game.isSelectingRect())
			{
				Game & game = Game::get();
				const Tiles & levelTiles = game.m_levelTiles;
				const Tiles & decalTiles = game.m_decalTiles;
				const Tiles & objectTiles = game.m_objectTiles;

				for (u32 y = 0; y < selectionCopy.size().y; ++y)
				{
					for (u32 x = 0; x < selectionCopy.size().x; ++x)
					{
						const auto & cell = selectionCopy.get({ x, y });

						if (0 != cell.tile.index)
						{
							Visual visual;
							Level::drawCellTile(_surface, levelTiles, cell, visual, x + m_mouseOverTile.x, y + m_mouseOverTile.y, 128);
						}

						if (0 != cell.decal.index)
						{
							Visual visual;
							Level::drawCellDecal(_surface, decalTiles, cell, visual, x + m_mouseOverTile.x, y + m_mouseOverTile.y, 128);
						}

						if (0 != cell.obj.index)
						{
							Visual visual;
							Level::drawCellObject(_surface, objectTiles, cell, visual, x + m_mouseOverTile.x, y + m_mouseOverTile.y, 128);
						}
					}
				}

				RectangleShape rect;
							   rect.setOutlineColor(borderColor);
							   rect.setPosition(Vector2f((float)(m_mouseOverTile.x * game.m_levelTiles.m_size.x + 2), (float)(m_mouseOverTile.y * game.m_levelTiles.m_size.y + 2)));
							   rect.setSize(Vector2f((float)((selectionCopy.size().x) * game.m_levelTiles.m_size.x - 4), (float)((selectionCopy.size().y) * game.m_levelTiles.m_size.y - 4)));
							   rect.setOutlineColor(Color(255, 255, 255, 64));
							   rect.setFillColor(Color(0, 0, 0, 0));
							   rect.setOutlineThickness(2.0f);

				_surface.draw(rect);
			}
		}
		else
		{
			if (draw)
			{
				auto cell = m_cells.get(m_mouseOverTile);
				
				Visual brush;
					   brush.setOrigin((Vector2f)origin);
					   brush.m_tileSet = tileSet;
					   brush.setImage(selectedTile, asBool(info.flags & CellFlags::FlipX), asBool(info.flags & CellFlags::FlipY));
					   brush.setPosition(brushPos);
					   brush.setColor(Color(255, 255, 255, 128));

				if (info.palette != 0)
				{
					brush.setLutIndex(info.palette);
					brush.setShaderID(game.m_visualShader);
				}

				brush.draw(_surface);
			}

			RectangleShape rect;
						   rect.setOutlineColor(borderColor);
						   rect.setPosition(Vector2f((float)(curCell.x * game.m_levelTiles.m_size.x + 2), (float)(curCell.y * game.m_levelTiles.m_size.y + 2)));
						   rect.setSize(Vector2f((float)(game.m_levelTiles.m_size.x - 4), (float)(game.m_levelTiles.m_size.y - 4)));
						   rect.setOutlineColor(Color(255, 255, 255, 128));
						   rect.setFillColor(Color(0, 0, 0, 0));
						   rect.setOutlineThickness(2.0f);

			_surface.draw(rect);
		}
	}

	return true;
}

//--------------------------------------------------------------------------
uint Level::getBgTileCategoryMask(const CellInfo & _cell) const
{
	uint mask = 0;

	const auto & tileInfo = m_backgroundTiles->m_tileInfos.get((uint)_cell.tile.index);
	const auto & decalInfo = m_decalTiles->m_tileInfos.get((uint)_cell.decal.index);

	if (_cell.tile.index > 0)
	{
		mask |= 1 << (uint)BgTileCategory::Ground;
	}

	if (_cell.decal.index > 0)
	{
		if (asBool(decalInfo.m_flags & TileFlags::Shadow))
			mask |= 1 << (uint)BgTileCategory::Shadow;
		else
			mask |= 1 << (uint)BgTileCategory::Decal;
	}

	return mask;
}

//--------------------------------------------------------------------------
void Level::buildTilesVertexArray()
{
	Game & game = Game::get();
	const auto cellCount = m_cells.size();

	static uint buildTilesVertexArrayCounter = 0;
	//debugPrint("buildTilesVertexArray %u\n", buildTilesVertexArrayCounter++);

	uint total[] =
	{
		0,0,0
	};
	static_assert(COUNT_OF(total) == (uint)BgTileCategory::Count);

	for (uint j = 0; j < cellCount.y; ++j)
	{
		for (uint i = 0; i < cellCount.x; ++i)
		{
			const CellInfo & cell = get({ i, j });

			const auto mask = getBgTileCategoryMask(cell);

			for (uint k = 0; k < (uint)BgTileCategory::Count; ++k)
			{
				if (mask & (1 << k))
					total[k]++;
			}
		}
	}

	for (uint k = 0; k < (uint)BgTileCategory::Count; ++k)
	{
		BgTileCategory category = (BgTileCategory)k;
		auto & va = m_tilesVertexArray[k];
		const auto count = total[(uint)category];

		if (0 == count)
			continue;

		const Tiles * tiles = nullptr;
		CellDataIndex index = (CellDataIndex)-1;

		switch (category)
		{
			default:
				assert(false);
				break;

			case BgTileCategory::Ground:
				tiles = m_backgroundTiles;
				index = CellDataIndex::Tile;
				break;
			
			case BgTileCategory::Decal:
				tiles = m_decalTiles;
				index = CellDataIndex::Decal;
				break;	

			case BgTileCategory::Shadow:
				tiles = m_decalTiles;
				index = CellDataIndex::Decal;
				break;
		}

		va.resize(count * 4);
		va.setPrimitiveType(PrimitiveType::Quads);

		const auto cellSize = tiles->m_size;
		const auto & tileCount = tiles->count();

		Vertex * __restrict vb = &va[0];

		uint adress = 0;

		for (u32 j = 0; j < cellCount.y; ++j)
		{
			for (u32 i = 0; i < cellCount.x; ++i)
			{
				const CellInfo & cell = get({ i, j });

				const auto cat = getBgTileCategoryMask(cell);

				if ((cat & (1<<k)) == 0)
					continue;

				const auto & data = cell.data[(uint)index];
				auto coords = tiles->indexToCoords(data.index);
				auto rect = tiles->getTileRect(coords);
				auto & info = tiles->m_tileInfos.get(coords);

				const auto offset = data.offset + Vector2i8(info.m_box.left+info.m_origin.x, info.m_box.top+info.m_origin.y);

				float u0 = rect.left;
				float v0 = rect.top;
				float u1 = rect.left + rect.width;
				float v1 = rect.top + rect.height;

				if (asBool(data.flags & CellFlags::FlipX))
					exchange(u0, u1);

				if (asBool(data.flags & CellFlags::FlipY))
					exchange(v0, v1);

				vb[adress + 0].position = { (float)((i + 0) * cellSize.x + offset.x), (float)((j + 0) * cellSize.y + offset.y) };
				vb[adress + 0].texCoords = { u0, v0 };													   
				vb[adress + 0].color = { 255,255,255, data.palette };

				vb[adress + 1].position = { (float)((i + 0) * cellSize.x + rect.width + offset.x), (float)((j + 0) * cellSize.y + offset.y) };
				vb[adress + 1].texCoords = { u1, v0 };
				vb[adress + 1].color = { 255,255,255, data.palette };

				vb[adress + 2].position = { (float)((i + 0) * cellSize.x + rect.width + offset.x), (float)((j + 0) * cellSize.y + rect.height + offset.y) };
				vb[adress + 2].texCoords = { u1, v1 };
				vb[adress + 2].color = { 255,255,255, data.palette };

				vb[adress + 3].position = { (float)((i + 0) * cellSize.x + offset.x), (float)((j + 0) * cellSize.y + rect.height + offset.y) };
				vb[adress + 3].texCoords = { u0, v1 };
				vb[adress + 3].color = { 255,255,255, data.palette };

				adress += 4;
			}
		}
	}
}

//--------------------------------------------------------------------------
bool Level::draw(Viewport & _viewport, BgTileCategory _category)
{
	auto & surface = _viewport.getSurface();
	Game & game = Game::get();

	switch (_category)
	{
		case BgTileCategory::Ground:
		{
			sf::Shader * forward_shader = ::Shader::get(game.m_tileShader);
					     forward_shader->setUniform("texture", m_backgroundTiles->m_texture);
					     forward_shader->setUniform("lut", game.m_spriteLut);

			sf::RenderStates forward;
							 forward.texture = &m_backgroundTiles->m_texture;
							 forward.shader = forward_shader;

			surface.draw(m_tilesVertexArray[(uint)BgTileCategory::Ground], forward);
		}
		break;

		case BgTileCategory::Decal:
		{
			sf::Shader * decal_shader = ::Shader::get(game.m_decalShader);
						 decal_shader->setUniform("texture", m_decalTiles->m_texture);
						 decal_shader->setUniform("lut", game.m_spriteLut);

			sf::RenderStates decals;
							 decals.texture = &m_decalTiles->m_texture;
							 decals.shader = decal_shader;

			surface.draw(m_tilesVertexArray[(uint)BgTileCategory::Decal], decals);
		}
		break;

		case BgTileCategory::Shadow:
		{
			sf::Shader * shadow_shader = ::Shader::get(game.m_shadowShader);
						 shadow_shader->setUniform("texture", m_decalTiles->m_texture);
						 shadow_shader->setUniform("instance", false);

			sf::RenderStates shadow;
							 shadow.texture = &m_decalTiles->m_texture;
							 shadow.shader = shadow_shader;
							 shadow.blendMode = game.m_shadowBlendMode;

			surface.draw(m_tilesVertexArray[(uint)BgTileCategory::Shadow], shadow);
			break;
		}
	}

	return true;
}

//--------------------------------------------------------------------------
void Level::getVisibleTilesRange(Vector2u & _min, Vector2u & _max, const Tiles & _tiles, const Viewport & _viewport) const
{
	const Vector2f minCorner = _viewport.getMinScreenCorner();
	const Vector2f maxCorner = _viewport.getMaxScreenCorner();

	_min = Vector2u( max((uint)(minCorner.x / _tiles.m_size.x), 0U), max((uint)(minCorner.y / _tiles.m_size.y), 0U));
	_max = Vector2u( min((uint)((maxCorner.x + _tiles.m_size.x - 1) / _tiles.m_size.x), m_cells.size().x),  min((uint)((maxCorner.y + _tiles.m_size.y - 1) / _tiles.m_size.y), m_cells.size().y));
}

//--------------------------------------------------------------------------
bool Level::drawTilesCollisions(Viewport & _viewport)
{
	Game & game = Game::get();

	if (Game::get().m_debugDisplay)
	{
		const Tiles & tiles = game.m_levelTiles;

		Vector2u minCell, maxCell;
		getVisibleTilesRange(minCell, maxCell, tiles, _viewport);

		auto & surface = _viewport.getSurface();

		for (u32 j = minCell.y; j < maxCell.y; ++j)
		{
			for (u32 i = minCell.x; i < maxCell.x; ++i)
			{
				const CellInfo & cell = get({ i, j });
				drawCellTileCollision(surface, tiles, cell, i, j);
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------
bool Level::drawObjectsCollisions(Viewport & _viewport)
{
	Game & game = Game::get();

	if (Game::get().m_debugDisplay)
	{
		const Tiles & tiles = game.m_objectTiles;

		Vector2u minCell, maxCell;
		getVisibleTilesRange(minCell, maxCell, tiles, _viewport);

		auto & surface = _viewport.getSurface();

		for (u32 j = minCell.y; j < maxCell.y; ++j)
		{
			for (u32 i = minCell.x; i < maxCell.x; ++i)
			{
				const CellInfo & cell = get({ i, j });
				drawObjectCollision(surface, tiles, cell, i, j);
			}
		}
	}

	return true;
}

//--------------------------------------------------------------------------
inline void Level::drawCellTileCollision(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, u32 _x, u32 _y, u32 _opacity)
{
	const Game & game = Game::get();
	if (game.isTileColliding(_cell.tile.index))
	{
		const auto coords = _tiles.indexToCoords(_cell.tile.index);
		const TileInfo & info = _tiles.m_tileInfos.get(coords);

		if (asBool(TileFlags::Collide & info.m_flags))
		{
			AABB aabb(Vector2f(_x * _tiles.m_size.x + 15 + _cell.tile.offset.x, _y * _tiles.m_size.y + 15 + _cell.tile.offset.y), info.m_collision);
			aabb.draw(_surface, { 187, 187, 255,128 });
		}
	}
}

//--------------------------------------------------------------------------
inline void Level::drawObjectCollision(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, u32 _x, u32 _y, u32 _opacity)
{
	const Game & game = Game::get();
	if (game.isTileObjectColliding(_cell.obj.index))
	{
		const auto coords = _tiles.indexToCoords(_cell.obj.index);
		const TileInfo & info = _tiles.m_tileInfos.get(coords);

		if (asBool(TileFlags::Collide & info.m_flags))
		{
			AABB aabb(Vector2f(_x * _tiles.m_size.x + 15 + _cell.obj.offset.x, _y * _tiles.m_size.y + 15 + _cell.obj.offset.y), info.m_collision);
			aabb.draw(_surface, { 187,255,187,128 });
		}
	}
}

//--------------------------------------------------------------------------
bool Level::drawObjects(Viewport & _viewport)
{
	Game & game = Game::get();
	
	m_visuals.clear();
	
	const Tiles & tiles = game.m_objectTiles;

	Vector2f minCorner = _viewport.getMinScreenCorner();
	Vector2f maxCorner = _viewport.getMaxScreenCorner();

	uint cellMinX = max((uint)(minCorner.x / game.m_levelTiles.m_size.x), 0U);
	uint cellMinY = max((uint)(minCorner.y / game.m_levelTiles.m_size.y), 0U);

	uint cellMaxX = min((uint)((maxCorner.x + game.m_levelTiles.m_size.x - 1) / game.m_levelTiles.m_size.x), m_cells.size().x);
	uint cellMaxY = min((uint)((maxCorner.y + game.m_levelTiles.m_size.y - 1) / game.m_levelTiles.m_size.y), m_cells.size().y);
	
	auto & surface = _viewport.getSurface();

	for (uint j = cellMinY; j < cellMaxY; ++j)
	{
		for (uint i = cellMinX; i < cellMaxX; ++i)
		{
			const CellInfo & cell = get({ i, j });
			const u8 obj = cell.obj.index;
	
			if (obj != 0)
			{
				Visual & visual = m_visuals.emplace_back();
				drawCellObject(surface, tiles, cell, visual, i, j, 255);
			}
		}
	}
	
	return true;
}

//--------------------------------------------------------------------------
template <class T> static void Level::drawCellInternal(sf::RenderTexture & _surface, const Tiles & _tiles, const T & _cell, Visual & _visual, u32 _x, u32 _y, u32 _opacity)
{
	const auto & game = Game::get();

	const auto coords = _tiles.indexToCoords(_cell.index);

	_visual.setTileSet(_tiles);
	_visual.setImage(coords, asBool(_cell.flags & CellFlags::FlipX), asBool(_cell.flags & CellFlags::FlipY));
	_visual.setPosition({ (float)(_x * _tiles.m_size.x + 15 + _cell.offset.x), (float)(_y * _tiles.m_size.y + 15 + _cell.offset.y) });
	_visual.setColor(Color(255, 255, 255, _opacity));

	if (_cell.palette != 0)
	{
		_visual.setLutIndex(_cell.palette);
		_visual.setShaderID(game.m_visualShader);
	}

	_visual.draw(_surface);
}

//--------------------------------------------------------------------------
inline void Level::drawCellDecal(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, Visual & _visual, u32 _x, u32 _y, u32 _opacity)
{
	drawCellInternal(_surface, _tiles, _cell.decal, _visual, _x, _y, _opacity);
}

//--------------------------------------------------------------------------
inline void Level::drawCellObject(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, Visual & _visual, u32 _x, u32 _y, u32 _opacity)
{
	drawCellInternal(_surface, _tiles, _cell.obj, _visual, _x, _y, _opacity);
}

//--------------------------------------------------------------------------
inline void Level::drawCellTile(sf::RenderTexture & _surface, const Tiles & _tiles, const CellInfo & _cell, Visual & _visual, u32 _x, u32 _y, u32 _opacity)
{
	drawCellInternal(_surface, _tiles, _cell.tile, _visual, _x, _y, _opacity);
}
