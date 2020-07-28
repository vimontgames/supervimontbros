//--------------------------------------------------------------------------
INLINE BallType Ball::getBallType() const 
{ 
	return m_ballType; 
}

//--------------------------------------------------------------------------
INLINE bool Ball::isFootBall() const
{
	switch (m_ballType)
	{
	case BallType::FootballWhite:
	case BallType::FootballYellow:
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
INLINE bool Ball::isRugbyBall() const
{
	switch (m_ballType)
	{
	case BallType::Rugby:
	case BallType::RugbyWhite:
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
INLINE bool Ball::isTennisBall() const
{
	switch (m_ballType)
	{
	case BallType::Tennis:
		return true;
	}

	return false;
}