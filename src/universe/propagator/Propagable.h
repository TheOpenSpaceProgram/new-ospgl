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

	// Any nullptr will be assumed to be n_body, must be same size as ligth states
	virtual TrajectoryVector* get_trajectories() = 0;
};