#pragma once
#include "PlanetarySystem.h"
#include "../vehicle/Vehicle.h"

// The Universe is the central class of the game. It stores both the system
// and everything else in the system (buildings and vehicles).
class Universe
{
public:

	PlanetarySystem system;
	std::vector<Vehicle*> vehicles;

	Universe();
	~Universe();
};

