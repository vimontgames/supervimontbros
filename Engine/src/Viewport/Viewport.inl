//--------------------------------------------------------------------------
INLINE sf::Vector2f Viewport::getMinScreenCorner() const
{
	return m_camOffset - 0.5f * (sf::Vector2f)m_size;
}

//--------------------------------------------------------------------------
INLINE sf::Vector2f Viewport::getMaxScreenCorner() const
{
	return m_camOffset + 0.5f * (sf::Vector2f)m_size;
}

//--------------------------------------------------------------------------
INLINE const sf::Vector2u Viewport::getSize() const
{
	return m_size;
}

//--------------------------------------------------------------------------
INLINE sf::RenderTexture & Viewport::getSurface()
{ 
	return *m_surface; 
}

//--------------------------------------------------------------------------
INLINE sf::View & Viewport::getView()
{ 
	return m_view; 
}

//--------------------------------------------------------------------------
INLINE sf::Vector2f & Viewport::getCameraOffset()
{ 
	return m_camOffset; 
}
//--------------------------------------------------------------------------
INLINE const sf::RenderTexture & Viewport::getSurface() const
{ 
	return *m_surface; 
}

//--------------------------------------------------------------------------
INLINE const sf::View & Viewport::getView() const
{ 
	return m_view; 
}

//--------------------------------------------------------------------------
INLINE const sf::Vector2f & Viewport::getCameraOffset() const
{ 
	return m_camOffset; 
}

//--------------------------------------------------------------------------
INLINE const sf::Vector2i & Viewport::getScreenPos() const
{ 
	return m_screenPos; 
}

//--------------------------------------------------------------------------
INLINE void Viewport::setCameraOffset(sf::Vector2f _offset)
{
	m_camOffset = _offset;
}

//--------------------------------------------------------------------------
INLINE bool Viewport::isEnabled() const
{
	return m_enabled;
}

//--------------------------------------------------------------------------
INLINE void Viewport::setEnable(bool _enable)
{
	m_enabled = _enable;
}

//--------------------------------------------------------------------------
INLINE std::vector<Visual*> & Viewport::getVisuals()
{ 
	return m_visualsToSort;
}

//--------------------------------------------------------------------------
INLINE std::vector<Visual*> & Viewport::getDecals()
{
	return m_decalsToSort;
}

//--------------------------------------------------------------------------
INLINE MessageList & Viewport::getMessageList()
{ 
	return m_messageList;
}

//--------------------------------------------------------------------------
INLINE PostProcess * Viewport::getPostProcess()
{ 
	return m_postProcess; 
}