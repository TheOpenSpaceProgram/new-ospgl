#pragma once
#include "../CartesianState.h"
#include <vector>

class Universe;

struct PredictedState
{
	// ABSOLUTE time
	double t;
	CartesianState st;
};

// Trajectories describe the spatial properties of objects
// (Position and velocity)
// Prediction is done externally to simplify the writing of 
// trajectories
class Trajectory
{
private:

	Universe* universe = nullptr;

public:
	
	// Some trajectories may also be used while bullet is loaded, 
	// that's what the optional flag is for
	virtual WorldState get_state(double t_now, bool use_bullet = false) = 0;

	virtual void init() {}

	void setup(Universe* universe)
	{
		this->universe = universe;
		init();
	}

	Universe* get_universe() { return universe; }
};