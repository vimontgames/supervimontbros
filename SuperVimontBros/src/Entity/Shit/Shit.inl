//--------------------------------------------------------------------------
INLINE bool Shit::isBallShit() const
{
	switch (m_shitType)
	{
		case ShitType::Ball:
		case ShitType::Rugby:
			return true;
	}

	return false;
}

//--------------------------------------------------------------------------
INLINE ShitType Shit::getShitType() const
{
	return m_shitType;
}