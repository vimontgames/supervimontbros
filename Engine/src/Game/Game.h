#pragma once

class Visual;

#include "Tiles/Tiles.h"
#include "Level/Level.h"

class Actor;
class Viewport;
class MessageList;
class PostProcess;

enum class EditorMode : u8
{
	None = 0,

	SelectTile,
	PaintTile,

	SelectDecal,
	PaintDecal,

	SelectObject,
	PaintObject,

	EditSprites,

	Count
};

enum class GameFont : u8
{
	Dlx8 = 0,
	PiXel5,

	Count
};

struct TryMoveResult
{
	bool canMoveX : 1;
	bool canMoveY : 1;

	explicit operator bool() const { return canMoveX || canMoveY; }
};

struct SpawnInfo
{
	SpawnInfo(Visual * _instance, Vector2i8 _offset = { 0, 0 }, Tiles * _tileSet = nullptr, u16 image = 0);

	Visual * instance;
	Vector2i8 offset;
	Tiles * tileSet;
	u16 image;
};

using SpawnList = std::vector<SpawnInfo>;

class Game
{
public:
	Game();
	virtual ~Game();

	virtual bool init(bool _reinit = false);
	virtual void reset();
	virtual void deinit();

	virtual bool run();
	virtual bool update();
	virtual bool draw();

	virtual bool updateShaders();
	virtual void updateViewports() = 0;

	static Game & get();

	sf::Vector2u levelSize() const;

	virtual void drawOverlay(sf::RenderTexture & _dst);

	bool drawFPS(sf::RenderTexture & _dst, bool _dt = false);
	virtual bool drawMode(sf::RenderTexture & _dst);
	bool drawTitle(sf::RenderTexture & _dst);
	bool drawDebugString(sf::RenderTexture & _dst);
	bool drawMouse(sf::RenderTexture & _dst);

	//--------------------------------------------------------------------------
	virtual void populateLevel();
	virtual void populateCell(SpawnList & _spawns, const CellInfo & _cell, uint _x, uint _y);

	virtual bool drawScore(Viewport & _viewport, sf::RenderTexture & _surface) = 0;
	virtual TryMoveResult tryMove(Actor * _actor, sf::Vector2f & _moveVec) const = 0;
	virtual bool isTileColliding(tileIndex _tile) const = 0;
	virtual bool isTileObjectColliding(objectIndex _obj) const = 0;

	bool isMouseOverScreen() const;
	bool inScreen(const sf::Vector2f & _position, bool _viewOffset = true) const;

	bool isMouseButtonPressed(sf::Mouse::Button _button) const;
	bool isMouseButtonJustPressed(sf::Mouse::Button _button) const;
	bool isMouseButtonJustReleased(sf::Mouse::Button _button) const;
	void setMouseButtonJustPressed(sf::Mouse::Button _button, bool _justPressed);

	bool acceptEditorInputs() const;
	bool canPressKey(float _delayMs = 150.0f) const;
	void setKeyPressed();

	void registerVisual(Visual * _visual);
	void unregisterVisual(Visual * _visual);

	const sf::Font & getFont(GameFont _font) const;

	bool zoomIn();
	bool zoomOut();

	Viewport & getDefaultViewport() const { return *m_viewports[0]; }

	void ReleaseAsync(Visual * _visual);

protected:
	void onUpdateScale();
	bool checkReinit();

public:
	static Game *					s_game;
	
	sf::RenderWindow				m_window;
	std::vector<Viewport*>			m_viewports;
	sf::String						m_name = "Game";
	sf::Vector2u					m_screenSize = { 320, 200 };
	bool							m_fullscreen = false;
	bool							m_splitScreen = true;
	u32								m_scale = 4;
	Tiles							m_spritesTile;
	sf::Font						m_font[(uint)GameFont::Count];
	bool							m_debugDisplay = false;
	sf::Clock						m_frameTimer;
	sf::Clock						m_keyPressTimer;
	float							m_dt = 0.0f;
	EditorMode						m_editorMode = EditorMode::None;
	bool							m_drawCollisions = false;
	sf::Sprite						m_mouse; 
	Tiles							m_levelTiles;
	Tiles							m_decalTiles;
	Level							m_level;
	Tiles							m_objectTiles;
	sf::String						m_debugString;
	bool							m_hasFocus = true;

	bool							m_mouseButtonPressed[(u32)sf::Mouse::ButtonCount] = { false, false, false, false, false };
	bool							m_mouseButtonJustPressed[(u32)sf::Mouse::ButtonCount] = { false, false, false, false, false };
	bool							m_mouseButtonJustReleased[(u32)sf::Mouse::ButtonCount] = { false, false, false, false, false };
	bool							m_mouseButtonReleased[(u32)sf::Mouse::ButtonCount] = { false, false, false, false, false };

#pragma region CopyPaste
public:
	bool							isSelectingRect() const;
	void							setSelectedRect(const sf::Vector2i & _min, const sf::Vector2i & _max);
	bool							getSelectedRect(sf::Vector2i & _min, sf::Vector2i & _max) const;
	void							resetSelectedRect();

private:
	bool							m_selectingRect = false;
	sf::Vector2i					m_selectedRect[2] = { { -1, -1 }, {-1,-1} };
#pragma endregion

public:
	ObjectList<Visual*>				m_visuals;
	ObjectList<Visual*>				m_decals;
	ObjectList<Actor*>				m_actors;

	MessageList * 					m_messageList = nullptr;
	PostProcess	*					m_postProcess = nullptr;

	sf::Texture						m_spriteLut;
	uint							m_spriteLutCount = 0;
	ShaderID						m_tileShader   = (ShaderID)-1;
	ShaderID						m_visualShader = (ShaderID)-1;
	ShaderID						m_shadowShader = (ShaderID)-1;
	ShaderID						m_decalShader  = (ShaderID)-1;

	sf::BlendMode					m_shadowBlendMode;

private:
	ObjectList<Visual*>				m_visualsToDelete;
	ObjectList<Visual*>				m_decalToDelete;
};