//--------------------------------------------------------------------------
INLINE PlayerType Player::getPlayerType() const
{
	return m_playerType;
}

//--------------------------------------------------------------------------
INLINE u8 Player::getPlayerTypeSubIndex() const
{
	return m_playerTypeIndex;
}

//--------------------------------------------------------------------------
INLINE const Controller & Player::getController() const
{
	return Controller::getController(m_controllerID);
}