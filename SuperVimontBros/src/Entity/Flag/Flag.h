#pragma once

#include "Entity/Entity.h"

enum class FlagType : u8
{
	YellowRed = 0,
	France,

	Count
};

class Flag : public Entity
{
public:
	Flag(FlagType _flagType);
	void init() override;

private:
	FlagType m_flagType;

	using Super = Entity;
};