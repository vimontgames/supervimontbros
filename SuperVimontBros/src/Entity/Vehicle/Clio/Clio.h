#pragma once

#include "Entity/Vehicle/Vehicle.h"

enum class ClioType
{
	Blue = 0,
	Red,

	Count
};

class Clio : public Vehicle
{
public:
	Clio(ClioType _clioType);
	~Clio();

	void init() override;
	void updateAABB() override;

private:
	ClioType m_clioType;
};