//--------------------------------------------------------------------------
INLINE EntityGUID Entity::getGUID() const
{
	return m_GUID;
}

//--------------------------------------------------------------------------
INLINE void Entity::setIsPickable(bool _pickable)
{
	m_isPickable = _pickable;
}

//--------------------------------------------------------------------------
INLINE bool Entity::isPickable() const 
{ 
	return m_isPickable; 
}

//--------------------------------------------------------------------------
INLINE bool Entity::isPicked() const
{ 
	return m_isPicked; 
}

//--------------------------------------------------------------------------
INLINE void Entity::setPicked(bool _isPicked)
{ 
	m_isPicked = _isPicked; 
}

//--------------------------------------------------------------------------
INLINE const AABB * Entity::getLastDropZoneAABB() const
{
	return m_lastDropZone;
}

//--------------------------------------------------------------------------
INLINE bool Entity::setLastDropZoneAABB(AABB * _dropZoneAABB)
{
	if (m_lastDropZone != _dropZoneAABB)
	{
		m_lastDropZone = _dropZoneAABB;
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------
INLINE bool Entity::isCarrying() const
{
	return m_pickedEntity != nullptr;
}

//--------------------------------------------------------------------------
INLINE bool Entity::pickEntity(Entity * _picked)
{
	if (_picked != m_pickedEntity)
	{
		_picked->setPicked(true);
		_picked->setParent(this);

		assert(_picked->m_isColliderForOtherActors);
		_picked->m_isColliderForOtherActors = false;

		m_pickedEntity = _picked;
	
		return true;
	}
	
	return false;
}

//--------------------------------------------------------------------------
INLINE const Entity * Entity::getCurrentlyPickedEntity() const
{
	return m_pickedEntity;
}