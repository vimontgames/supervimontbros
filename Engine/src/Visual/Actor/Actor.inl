//--------------------------------------------------------------------------
INLINE void Actor::setParent(Visual * _parent)
{
	if (_parent != m_parent)
	{
		SAFE_RELEASE(m_parent);
		SAFE_INCREASE_REFCOUNT(_parent);
		m_parent = _parent;
	}
}

//--------------------------------------------------------------------------
INLINE Visual * Actor::getParent() const
{
	return m_parent;
}

//--------------------------------------------------------------------------
INLINE bool Actor::isMoving() const
{ 
	return length(m_velocity) > 0.01f; 
}