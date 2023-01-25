#pragma once
#include "../UniverseDefinitions.h"

class Propagable
{
public:

	// Returned vector will be propagated. Only called once
	// Tolerates changes of length to the vector
	virtual StateVector* get_massful_states() = 0;
	// Returned vector will be propagated. Only called once
	// Tolerates changes of length to the vector
	virtual LightStateVector* get_light_states() = 0;
};