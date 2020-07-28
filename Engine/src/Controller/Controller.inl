//--------------------------------------------------------------------------
INLINE float Controller::getXAxis() const
{
	return m_state.x;
}

//--------------------------------------------------------------------------
INLINE float Controller::getYAxis() const
{
	return m_state.y;
}

//--------------------------------------------------------------------------
INLINE sf::Vector2f Controller::getXYAxis() const
{
	return { m_state.x, m_state.y };
}

//--------------------------------------------------------------------------
INLINE bool Controller::isButtonPressed(uint _index) const
{
	assert(_index < getButtonCount());
	return m_state.buttonPressed[_index];
}

//--------------------------------------------------------------------------
INLINE bool Controller::isButtonJustPressed(uint _index) const
{
	assert(_index < getButtonCount());
	return m_state.buttonJustPressed[_index];
}

//--------------------------------------------------------------------------
INLINE bool Controller::isButtonJustReleased(uint _index) const
{
	assert(_index < getButtonCount());
	return m_state.buttonJustReleased[_index];
}

//--------------------------------------------------------------------------
INLINE uint Controller::getButtonCount()
{
	return COUNT_OF(m_state.buttonPressed);
}