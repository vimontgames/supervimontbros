#pragma once

#include "Object/Object.h"
#include "Message/MessageList.h"

class Visual;
class PostProcess;
struct TileInfo;

class Viewport
{
public:
	Viewport(const char * _name);
	~Viewport();

	void init(u32 _width, u32 _height, i32 _offsetX = 0, i32 _offsetY = 0);
	void reset();

	void update(bool _addVisuals, const ObjectList<Visual*> & _visuals, bool _addDecals, const ObjectList<Visual*> _decals);

	void begin();
	void end();

	bool isVisible(const Visual * _visual) const;
	bool isVisible(sf::Vector2f _point, const TileInfo & _tileInfo) const;
	bool isVisible(sf::Vector2f _point, float x = 0, float y = 0, float X = 0, float Y = 0) const;

	const sf::Vector2u getSize() const;

	sf::Vector2f getMinScreenCorner() const;
	sf::Vector2f getMaxScreenCorner() const;	

	void setCameraOffset(sf::Vector2f _offset);

	sf::RenderTexture & getSurface();
	sf::View & getView();
	sf::Vector2f & getCameraOffset();
	const sf::RenderTexture & getSurface() const;
	const sf::View & getView() const;
	const sf::Vector2f & getCameraOffset() const;
	const sf::Vector2i & getScreenPos() const;
	bool isEnabled() const;
	void setEnable(bool _enable);
	std::vector<Visual*> & getVisuals();
	std::vector<Visual*> & getDecals();
	MessageList & getMessageList();
	PostProcess * getPostProcess();

private:
	char *					m_name = nullptr;
	sf::Vector2u			m_size = { 0,0 };
	sf::RenderTexture *		m_surface = nullptr;
	sf::View				m_view;
	sf::Vector2f			m_camOffset = { 0,0 };
	sf::Vector2i			m_screenPos = { 0, 0 };
	bool					m_enabled = true;
	std::vector<Visual*>	m_visualsToSort;
	std::vector<Visual*>	m_decalsToSort;
	MessageList				m_messageList;
	PostProcess *			m_postProcess = nullptr;
};

#ifdef ENABLE_INL
#include "Viewport.inl"
#endif