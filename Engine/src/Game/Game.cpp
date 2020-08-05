#include "Precomp.h"
#include "Game.h"
#include "PostProcess/PostProcess.h"
#include "Visual/Actor/Actor.h"
#include "Viewport/Viewport.h"
#include "Message/MessageList.h"
#include "PostProcess/PostProcess.h"
#include "Controller/Controller.h"
#include "Shader/Shader.h"
#include "RenderTarget/RenderTarget.h"

Game * Game::s_game = nullptr;

using namespace sf;

//--------------------------------------------------------------------------
Game::Game()
{
	srand(time(NULL));

	s_game = this;

	m_postProcess = new PostProcess();
	m_messageList = new MessageList();
}

//--------------------------------------------------------------------------
Game::~Game()
{
	s_game = nullptr;

	for (Viewport * vp : m_viewports)
	{
		SAFE_DELETE(vp);
	}
	m_viewports.clear();

	SAFE_DELETE(m_postProcess);
	SAFE_DELETE(m_messageList);

	// Check Object leaks
	uint count = Object::getObjectCount();
	if (0 != count)
	{
		debugPrint("%u Object not released\n", count);
	}
}

//--------------------------------------------------------------------------
Game & Game::get()
{
	return *s_game;
}

//--------------------------------------------------------------------------
bool Game::init(bool _reinit)
{
	if (!_reinit)
	{
		// Default shader
		::Shader::init();

		// Temp surfaces
		RenderTargetManager::init();

		// shaders
		m_tileShader   = ::Shader::add("Engine/data/shader/tile_vs.fx", "Engine/data/shader/tile_ps.fx");
		m_visualShader = ::Shader::add("Engine/data/shader/visual_vs.fx", "Engine/data/shader/visual_ps.fx");
		m_shadowShader = ::Shader::add("Engine/data/shader/shadow_vs.fx", "Engine/data/shader/shadow_ps.fx");
		m_decalShader  = ::Shader::add("Engine/data/shader/decal_vs.fx", "Engine/data/shader/decal_ps.fx");

		// render state
		m_shadowBlendMode = BlendMode(BlendMode::Factor::SrcColor, BlendMode::Factor::OneMinusSrcColor, BlendMode::Equation::ReverseSubtract);

		// Create window
		m_window.create(VideoMode(m_screenSize.x * m_scale, m_screenSize.y * m_scale), m_name, m_fullscreen ? Style::Fullscreen : Style::Default);

		// Create editor viewport
		m_viewports.push_back(new Viewport("Editor"));
		m_viewports[0]->init(m_screenSize.x, m_screenSize.y);

		// Create default game viewport
		m_viewports.push_back(new Viewport("Game #1"));
		m_viewports[1]->init(m_screenSize.x, m_screenSize.y);
	}

	m_postProcess->init();
	m_messageList->clear();

	// Load fonts
	m_font[(uint)GameFont::Dlx8].loadFromFile("SuperVimontBros/data/font/dlxfont.ttf");
	m_font[(uint)GameFont::PiXel5].loadFromFile("SuperVimontBros/data/font/piXelize.ttf");

	// Mouse
	TileInfo & info = m_spritesTile.m_tileInfos.get({ 0,15 });

	m_mouse.setTexture(m_spritesTile.m_texture);
	m_mouse.setTextureRect(m_spritesTile.getTileRect({ 0, 15 }));
	m_mouse.setOrigin((Vector2f)info.m_origin - Vector2f((float)(info.m_box.left + 15), (float)(info.m_box.top + 15)));

	// Reset game camera
	for (Viewport * vp : m_viewports)
		vp->reset();

	// Reset mode
	m_editorMode = EditorMode::None;
	
	// release actors
	reset();

	// init controllers
	Controller::init();

	resetSelectedRect();

	return true;
}

//--------------------------------------------------------------------------
void Game::reset()
{
	m_visuals.clear();
	m_decals.clear();
	m_actors.clear();
}

//--------------------------------------------------------------------------
void Game::deinit()
{
	reset();

	RenderTargetManager::deinit();
	Controller::deinit();
	::Shader::deinit();
}

//--------------------------------------------------------------------------
void Game::registerVisual(Visual * _visual)
{
	if (_visual->m_isDecal)
		m_decals.add(_visual);
	else
		m_visuals.add(_visual);

	Actor * actor = dynamic_cast<Actor*>(_visual);
	if (actor)
		m_actors.add(actor);
}

//--------------------------------------------------------------------------
void Game::unregisterVisual(Visual * _visual)
{
	Actor * actor = dynamic_cast<Actor*>(_visual);
	if (actor)
		m_actors.remove(actor);

	if (_visual->m_isDecal)
		m_decals.remove(_visual);
	else
		m_visuals.remove(_visual);
}

//--------------------------------------------------------------------------
bool Game::isMouseButtonPressed(sf::Mouse::Button _button) const
{
	return m_mouseButtonPressed[(u32)_button];
}

//--------------------------------------------------------------------------
bool Game::isMouseButtonJustPressed(sf::Mouse::Button _button) const
{
	return m_mouseButtonJustPressed[(u32)_button];
}

//--------------------------------------------------------------------------
bool Game::isMouseButtonJustReleased(sf::Mouse::Button _button) const
{
	return m_mouseButtonJustReleased[(u32)_button];
}

//--------------------------------------------------------------------------
void Game::setMouseButtonJustPressed(sf::Mouse::Button _button, bool _justPressed)
{
	m_mouseButtonJustPressed[(u32)_button] = _justPressed;
}

//--------------------------------------------------------------------------
void Game::ReleaseAsync(Visual * _visual)
{
	if (_visual->m_isDecal)
		m_decalToDelete.add(_visual);
	else
		m_visualsToDelete.add(_visual);
}

//--------------------------------------------------------------------------
bool Game::update()
{
	Controller::update();

	// Visuals to release
	for (uint i = 0; i < m_visualsToDelete.size(); ++i)
	{
		Game::get().unregisterVisual(m_visualsToDelete[i]);
	}
	m_visualsToDelete.clear();

	for (uint i = 0; i < m_decalToDelete.size(); ++i)
	{
		Game::get().unregisterVisual(m_decalToDelete[i]);
	}
	m_decalToDelete.clear();

	// Show/Hide viewports
	const bool gameMode = m_editorMode == EditorMode::None;

	for (Viewport * viewport : m_viewports)
	{
		if (viewport != &getDefaultViewport())
			viewport->setEnable(gameMode);
		else
			viewport->setEnable(!gameMode);

		viewport->update(gameMode, m_visuals, gameMode, m_decals);
	}

	// Update logic
	if (gameMode)
	{
		m_visuals.update(m_dt);
		m_decals.update(m_dt);
	}

	bool resetKeyPressTimer = false;

	if (acceptEditorInputs())
	{
		if (m_debugDisplay)
		{
			if (Keyboard::isKeyPressed(Keyboard::Key::Add))
			{
				zoomIn();
				setKeyPressed();
			}
			else if (Keyboard::isKeyPressed(Keyboard::Key::Subtract))
			{
				zoomOut();
				setKeyPressed();
			}
		}

		// Debug mode ON/OFF
		//if (Mode::GameMode == m_mode)
		{
			if (Keyboard::isKeyPressed(Keyboard::Key::F5))
			{
				m_debugDisplay = !m_debugDisplay;
				setKeyPressed();

				if (!m_debugDisplay)
					m_editorMode = EditorMode::None;
			}
		}

		if (isMouseOverScreen())
		{
			m_window.setMouseCursorVisible(false);

			if (Keyboard::isKeyPressed(Keyboard::Key::Escape))
			{
				switch (m_editorMode)
				{
					case EditorMode::SelectTile:
						m_editorMode = EditorMode::PaintTile;
						setKeyPressed();
						break;

					case EditorMode::SelectDecal:
						m_editorMode = EditorMode::PaintDecal;
						setKeyPressed();
						break;

					case EditorMode::SelectObject:
						m_editorMode = EditorMode::PaintObject;
						setKeyPressed();
						break;

					case EditorMode::PaintTile:
					case EditorMode::PaintObject:
					case EditorMode::PaintDecal:
					case EditorMode::EditSprites:
						m_editorMode = EditorMode::None;
						setKeyPressed();
						break;
				}
			}
		}
		else
		{
			m_window.setMouseCursorVisible(true);
		}
	}

	if (m_fullscreen)
	{
		m_window.setMouseCursorVisible(false);
	}

	// Toggle mode
	if (m_debugDisplay)
	{
		for (uint b = 0; b < (u32)Mouse::ButtonCount; ++b)
		{
			bool pressed = Mouse::isButtonPressed((Mouse::Button)b);

			if (pressed == true)
			{
				if (m_mouseButtonPressed[b] == false)
				{
					m_mouseButtonJustPressed[b] = true;
				}
				else
				{
					m_mouseButtonJustPressed[b] = false;
				}
			}
			else
			{
				if (m_mouseButtonPressed[b] == true)
				{
					m_mouseButtonJustReleased[b] = true;
				}
				else
				{
					m_mouseButtonJustReleased[b] = false;
				}
			}

			m_mouseButtonPressed[b] = pressed;
		}

		if (isMouseOverScreen())
		{
			const Vector2f mousePos = m_mouse.getPosition();
			const Vector2u size = m_levelTiles.m_size;

			if (isMouseButtonJustPressed(Mouse::Button::Right))
			{
				switch (m_editorMode)
				{
					case EditorMode::None:
					case EditorMode::PaintTile:
						m_editorMode = EditorMode::SelectTile;
						setMouseButtonJustPressed(Mouse::Button::Right, false);
						break;

					case EditorMode::SelectTile:
						m_editorMode = EditorMode::SelectDecal;
						setMouseButtonJustPressed(Mouse::Button::Right, false);
						break;

					case EditorMode::PaintObject:
						m_editorMode = EditorMode::SelectObject;
						setMouseButtonJustPressed(Mouse::Button::Right, false);
						break;

					case EditorMode::SelectDecal:
						m_editorMode = EditorMode::SelectObject;
						setMouseButtonJustPressed(Mouse::Button::Right, false);
						break;

					case EditorMode::PaintDecal:
						m_editorMode = EditorMode::SelectDecal;
						setMouseButtonJustPressed(Mouse::Button::Right, false);
						break;

					case EditorMode::SelectObject:
						m_editorMode = EditorMode::EditSprites;
						setMouseButtonJustPressed(Mouse::Button::Right, false);
						break;

					case EditorMode::EditSprites:
						m_editorMode = EditorMode::None;
						setMouseButtonJustPressed(Mouse::Button::Right, false);
						break;
				}
			}

			// Draw selection update & load/save
			switch (m_editorMode)
			{
				case EditorMode::SelectTile:
					if (m_levelTiles.updateSelectTiles())
						return true;
					break;

				case EditorMode::SelectDecal:
					if (m_decalTiles.updateSelectTiles())
						return true;
					break;

				case EditorMode::SelectObject:
					if (m_objectTiles.updateSelectTiles())
						return true;
					break;

				case EditorMode::EditSprites:
					if (m_spritesTile.updateSelectTiles())
						return true;
					break;

				case EditorMode::PaintTile:
				case EditorMode::PaintDecal:
				case EditorMode::PaintObject:
					if (Keyboard::isKeyPressed(Keyboard::LControl))
					{
						if (Keyboard::isKeyPressed(Keyboard::Key::L))
						{
							m_level.load();
							return true;
						}

						if (Keyboard::isKeyPressed(Keyboard::Key::S))
						{
							m_level.save();
							return true;
						}

						if (Mouse::isButtonPressed(Mouse::Button::Left))
						{
							Vector2f camOffset;
									 camOffset.x = m_level.m_paintCamera.x - m_screenSize.x / 2.0f;
									 camOffset.y = m_level.m_paintCamera.y - m_screenSize.y / 2.0f;

							if (!m_selectingRect)
							{
								m_selectedRect[0] = Vector2i(((mousePos.x + camOffset.x) / size.x), ((mousePos.y + camOffset.y) / size.y));
								m_selectedRect[1] = m_selectedRect[0];
								m_selectingRect = true;
							}
							else
							{
								m_selectedRect[1] = Vector2i(((mousePos.x + camOffset.x) / size.x), ((mousePos.y + camOffset.y) / size.y));
							}
						}
						else if (isMouseButtonJustReleased(Mouse::Button::Left) && m_selectingRect)
						{
							m_selectingRect = false;
						}
					}
					break;				
			}
		}
	}

	// update level
	m_level.update();
	
	return true;
}

//--------------------------------------------------------------------------
bool Game::draw()
{
	auto & defaultViewport = getDefaultViewport();

	// low-res backbuffer
	sf::RenderTexture * backbuffer = RenderTargetManager::get(RenderTargetDesc(defaultViewport.getView().getSize().x, defaultViewport.getView().getSize().y));
	backbuffer->clear({ 0,0,0,255 });

	// Render views to textures
	for (Viewport * viewportPtr : m_viewports)
	{
		auto & viewport = *viewportPtr;

		if (!viewport.isEnabled())
			continue;
		
		const bool isDefaultViewport = &viewport == &defaultViewport;

		viewport.begin();
		{
			auto & surface = viewport.getSurface();

			if (EditorMode::None == m_editorMode)
			{
				m_debugString += "\n\n"; // skip 2 lines for score
			}

			if (m_editorMode != EditorMode::SelectTile)
			{
				auto & visuals = viewport.getVisuals();
				auto & decals = viewport.getDecals();

				m_level.draw(viewport, BgTileCategory::Ground);
				m_level.draw(viewport, BgTileCategory::Decal);

				if (m_editorMode == EditorMode::None)
				{
					std::sort(decals.begin(), decals.end(), Actor::sortY);

					for (int i = 0; i < decals.size(); ++i)
					{
						Visual * decal = decals[i];
						decal->draw(surface);
					}
				}

				m_level.draw(viewport, BgTileCategory::Shadow);
	
				if (m_editorMode == EditorMode::PaintObject || m_editorMode == EditorMode::PaintTile || m_editorMode == EditorMode::PaintDecal)
				{
					m_level.drawObjects(viewport);
				}

				if (m_drawCollisions)
					m_level.drawTilesCollisions(viewport);

				if (m_editorMode == EditorMode::None)
				{
					std::sort(visuals.begin(), visuals.end(), Actor::sortY);

					for (int i = 0; i < visuals.size(); ++i)
					{
						Visual * visual = visuals[i];
						visual->drawShadow(surface);
					}

					for (int i = 0; i < visuals.size(); ++i)
					{
						Visual * visual = visuals[i];
						visual->draw(surface);
					}
				}

				if (EditorMode::None != m_editorMode)
				{
					if (m_drawCollisions)
						m_level.drawObjectsCollisions(viewport);
				}

				m_level.drawPainting(surface);
			}

			// Reset camera coordinates
			surface.setView(surface.getDefaultView());

			// do postprocess to temp surface for viewport
			surface.display();

			sf::RenderTexture * temp = RenderTargetManager::get(RenderTargetDesc(viewport.getView().getSize().x, viewport.getView().getSize().y));

			auto * postProcess = viewport.getPostProcess();
			postProcess->draw(surface, *temp, false);

			if (isDefaultViewport)
			{
				switch (m_editorMode)
				{
					case EditorMode::SelectTile:
						m_levelTiles.drawSelectTiles(*temp);
						break;

					case EditorMode::SelectObject:
						m_objectTiles.drawSelectTiles(*temp);
						break;

					case EditorMode::SelectDecal:
						m_decalTiles.drawSelectTiles(*temp);
						break;

					case EditorMode::EditSprites:
						m_spritesTile.drawSelectTiles(*temp);
						break;
				}
			}

			if (EditorMode::None == m_editorMode)
			{
				viewport.getMessageList().draw(*temp);
				drawScore(viewport, *temp);
			}

			// copy back to backbuffer & preserve alpha
			temp->display();
			sf::Sprite quad(temp->getTexture());
					   quad.setPosition((sf::Vector2f)viewport.getScreenPos());
			backbuffer->draw(quad, BlendMode(BlendMode::Factor::One, BlendMode::Factor::Zero, BlendMode::Add));

			// low-res backbuffer
			backbuffer->display();

			// is it safe to release before "backbuffer->display()" ?
			RenderTargetManager::release(temp);
		}
		viewport.end();
	}

	sf::RenderTexture * tempBackbuffer = RenderTargetManager::get(RenderTargetDesc(backbuffer->getSize().x, backbuffer->getSize().y));

	if (m_editorMode == EditorMode::None)
	{
		m_postProcess->draw(*backbuffer, *tempBackbuffer, true);
	}
	else
	{
		sf::Sprite quad(backbuffer->getTexture());
		tempBackbuffer->draw(quad, BlendMode(BlendMode::One, BlendMode::Zero, BlendMode::Add));
	}

	RenderTargetManager::release(backbuffer);

	drawOverlay(*tempBackbuffer);

	// Upscale default view to backbuffer
	m_window.clear(Color(0, 0, 0));

	tempBackbuffer->display();
	sf::Sprite quad(tempBackbuffer->getTexture());
	quad.setScale((float)m_scale, (float)m_scale);
	m_window.draw(quad);

	m_window.display();
	RenderTargetManager::release(tempBackbuffer);

	return true;
}

//--------------------------------------------------------------------------
void Game::drawOverlay(sf::RenderTexture & _dst)
{
	// Overlay
	if (m_debugDisplay)
	{
		drawMode(_dst);
		drawDebugString(_dst);
		drawMouse(_dst);
		drawFPS(_dst, true);
	}
	else
	{
		drawFPS(_dst);
	}

	m_messageList->draw(_dst);
}

//--------------------------------------------------------------------------
bool Game::updateShaders()
{
	return 0 == ::Shader::update();
}

//--------------------------------------------------------------------------
bool Game::zoomIn()
{
	if (m_scale < 6)
	{
		m_scale += 1;
		onUpdateScale();
		updateViewports();
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
bool Game::zoomOut()
{
	if (m_scale >= 2)
	{
		m_scale -= 1;
		onUpdateScale();
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
void Game::onUpdateScale()
{
	m_screenSize = { 1920 / m_scale, 1080 / m_scale };
	getDefaultViewport().init(m_screenSize.x, m_screenSize.y);

	updateViewports();
}

//--------------------------------------------------------------------------
bool Game::drawMode(sf::RenderTexture & _dst)
{
	if (EditorMode::None != m_editorMode)
	{
		//static const char * editorModeNames[] =
		//{
		//	"",
		//	"Choisir brique du décor\n",
		//	"Peindre brique du décor\n",
		//	"Choisir objet du décor\n",
		//	"Peindre objet du décor\n",
		//	"Editeur de sprites\n"
		//};
		//static_assert(COUNT_OF(editorModeNames) == (uint)EditorMode::Count);
		//
		//m_debugString = String(editorModeNames[(u32)m_editorMode]) + m_debugString + "\n";

		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------------------
const sf::Font & Game::getFont(GameFont _font) const
{
	return m_font[(uint)_font];
}

//--------------------------------------------------------------------------
bool Game::drawFPS(sf::RenderTexture & _dst, bool _dt)
{
	char fpsString[256];

	if (_dt)
	{
		sprintf_s(fpsString, "%.2f ms - %.0f FPS", m_dt, 1.0f / m_dt * 1000.0f);
	}
	else
	{
		sprintf_s(fpsString, "%.0f FPS", 1.0f / m_dt * 1000.0f);
	}

	Text fpsText;
		 fpsText.setString(fpsString);
		 fpsText.setFont(getFont(GameFont::Dlx8));
		 fpsText.setCharacterSize(8);
		 fpsText.setPosition(m_screenSize.x - strlen(fpsString) * 8, m_screenSize.y - 8);
		 fpsText.setFillColor(sf::Color::White);

	_dst.draw(fpsText);

	return true;
}

//--------------------------------------------------------------------------
bool Game::drawTitle(sf::RenderTexture & _dst)
{
	Text title("Super Vimont Bros", getFont(GameFont::Dlx8));
	title.setCharacterSize(16);
	title.setPosition(0, 4);
	title.setFillColor(sf::Color::White);

	_dst.draw(title);

	return true;
}

//--------------------------------------------------------------------------
bool Game::drawDebugString(sf::RenderTexture & _dst)
{
	Text debugText;
		 debugText.setString(m_debugString);
		 debugText.setFont(getFont(GameFont::Dlx8));
		 debugText.setCharacterSize(8);
		 debugText.setPosition(0, 0);
		 debugText.setFillColor(sf::Color::Black);

	_dst.draw(debugText);

	return true;
}

//--------------------------------------------------------------------------
bool Game::drawMouse(sf::RenderTexture & _dst)
{
	uint caption = 0;

#if defined(_WIN32) || defined(_WIN64) 
	caption = ::GetSystemMetrics(SM_CYCAPTION);
#endif

	// Handle resize
	Vector2u winSize = m_window.getSize();
	
	const float scaleMouseX = ((float)m_screenSize.x * m_scale) / (float)winSize.x;
	const float scaleMouseY = ((float)m_screenSize.y * m_scale) / (float)winSize.y;

	Vector2i windowPos = m_window.getPosition();
	Vector2f mousePos = Vector2f((float)(Mouse::getPosition().x - windowPos.x) / m_scale, (float)(Mouse::getPosition().y - windowPos.y - caption) / m_scale);
			 mousePos.x *= scaleMouseX;
			 mousePos.y *= scaleMouseY;

	m_mouse.setPosition(mousePos);
	_dst.draw(m_mouse);

	return true;
}

//--------------------------------------------------------------------------
bool Game::run()
{
	while (m_window.isOpen())
	{
		// reset debug string
		m_debugString.clear();

		m_dt = min(1000.0f/30.0f, m_frameTimer.getElapsedTime().asMicroseconds() / 1000.0f); // clamp max dt
		m_frameTimer.restart();

		Event event;
		while (m_window.pollEvent(event))
		{
			switch (event.type)
			{
			case Event::Closed:
				m_window.close();
				break;

			case Event::LostFocus:
				m_hasFocus = false;
				break;

			case Event::GainedFocus:
				m_hasFocus = true;
				break;
			}
		}

		// Update entities
		update();

		// Draw stuff
		draw();

		// Reinit
		checkReinit();
	}

	return true;
}

//--------------------------------------------------------------------------
bool Game::canPressKey(float _delayMs) const
{
	return m_keyPressTimer.getElapsedTime().asMilliseconds() >= _delayMs;
}

//--------------------------------------------------------------------------
bool Game::acceptEditorInputs() const
{
	return isMouseOverScreen() && canPressKey();
}

//--------------------------------------------------------------------------
void Game::setKeyPressed()
{
	m_keyPressTimer.restart();
}

//--------------------------------------------------------------------------
bool Game::checkReinit()
{
	if (canPressKey())
	{
		if (Keyboard::isKeyPressed(Keyboard::Key::F6))
		{
			init(true);
			setKeyPressed();
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------
bool Game::inScreen(const Vector2f & _position, bool _viewOffset) const
{
	IntRect rect = { 0, 0, (int)m_screenSize.x, (int)m_screenSize.y };

	if (_viewOffset)
	{
		auto & viewport = getDefaultViewport();
		rect.left += viewport.getView().getCenter().x - m_screenSize.x / 2;
		rect.top += viewport.getView().getCenter().y- m_screenSize.y / 2;
	}

	return _position.x >= rect.left && _position.x < (rect.left + rect.width) && _position.y >= rect.top && _position.y < (rect.top + rect.height);
}

//--------------------------------------------------------------------------
bool Game::isMouseOverScreen() const
{
	return inScreen(m_mouse.getPosition(), false) && m_hasFocus;
}

//--------------------------------------------------------------------------
bool Game::isSelectingRect() const
{
	return m_selectingRect;
}
 
//--------------------------------------------------------------------------
void Game::resetSelectedRect()
{
	m_selectedRect[0] = m_selectedRect[1] = { -1, -1 };
}

//--------------------------------------------------------------------------
void Game::setSelectedRect(const sf::Vector2i & _min, const sf::Vector2i & _max)
{
	m_selectedRect[0] = _min;
	m_selectedRect[1] = _max;
}

//--------------------------------------------------------------------------
bool Game::getSelectedRect(sf::Vector2i & _min, sf::Vector2i & _max) const
{
	_min = { min(m_selectedRect[0].x, m_selectedRect[1].x), min(m_selectedRect[0].y, m_selectedRect[1].y) };
	_max = { max(m_selectedRect[0].x, m_selectedRect[1].x), max(m_selectedRect[0].y, m_selectedRect[1].y) };

	if (-1 != m_selectedRect[0].x && -1 != m_selectedRect[0].y && -1 != m_selectedRect[1].x && -1 != m_selectedRect[1].y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//--------------------------------------------------------------------------
void Game::populateLevel()
{
	const uint w = (uint)m_levelTiles.m_size.x;
	const uint h = (uint)m_levelTiles.m_size.y;

	for (uint y = 0; y < m_level.m_cells.size().y; ++y)
	{
		for (uint x = 0; x < m_level.m_cells.size().x; ++x)
		{
			const CellInfo & cell = m_level.get({ x, y });
			const auto offset = (Vector2i)cell.obj.offset;

			SpawnList spawns;
			populateCell(spawns, cell, x, y);

			for (auto & spawnInfo : spawns)
			{
				auto * tiles = spawnInfo.tileSet;

				Visual *& visual = spawnInfo.instance;

				visual->setImage(tiles->indexToCoords(cell.obj.index));
				visual->setPosition((float)(x * w + (w - 1) / 2), (float)(y * h + (h - 1) / 2));
				visual->move((Vector2f)(spawnInfo.offset + cell.obj.offset));
				visual->setSpawnCellPos({x,y});
				if (cell.obj.palette != 0)
				{
					visual->setLutIndex(cell.obj.palette);
					visual->setShaderID(m_visualShader);
				}

				Actor * actor = dynamic_cast<Actor*>(visual);
				if (actor)
				{
					if (asBool(cell.obj.flags & CellFlags::FlipX))
					{
						actor->m_faceLeft = true;
						actor->m_animLeft = true;
					}
				}

				visual->init();
				
				registerVisual(spawnInfo.instance);
				SAFE_RELEASE(spawnInfo.instance);
			}
		}
	}
}

//--------------------------------------------------------------------------
SpawnInfo::SpawnInfo(Visual * _instance, Vector2i8 _offset, Tiles * _tileSet, u16 _image) :
	instance(_instance),
	offset(_offset),
	tileSet(_tileSet),
	image(_image)
{
	if (nullptr == tileSet)
	{
		tileSet = &Game::get().m_objectTiles;
	}
}

//--------------------------------------------------------------------------
//SpawnInfo::SpawnInfo(Visual * _instance, Vector2i8 _offset) :
//	instance(_instance),
//	offset(_offset)
//{
//	tileSet = &Game::get().m_objectTiles;
//}

//--------------------------------------------------------------------------
// Default is to spawn a Visual if the "TileFlags::Visual" flag is set
//--------------------------------------------------------------------------
void Game::populateCell(SpawnList & _spawns, const CellInfo & _cell, uint _x, uint _y)
{
	Vector2u objCoords = m_objectTiles.indexToCoords(_cell.obj.index);
	const auto & objInfo = m_objectTiles.m_tileInfos.get(objCoords);

	if (asBool(TileFlags::Visual & objInfo.m_flags))
	{
		char objName[16];
		sprintf_s(objName, "Visual%u_%u_%u", _cell.obj.index, objCoords.x, objCoords.y);

		_spawns.push_back({ new Visual(objName, m_objectTiles), {0,0}, &m_objectTiles, _cell.obj.index });
	}
}
