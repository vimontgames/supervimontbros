#pragma once

#include "Entity/Vehicle/Vehicle.h"

class Peugeot3008 : public Vehicle
{
public:
	Peugeot3008();
	~Peugeot3008();

	void init() override;
	void updateAABB() override;
};