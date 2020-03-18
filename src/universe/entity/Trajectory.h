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

public:

	virtual WorldState get_state(double t_now) = 0;
	// You may not want to use this if you are a non-integrated trajectory
	// (for example, landed vehicles can be determined from time directly)
	virtual void update(double dt) = 0;

	virtual void start(WorldState s0, double t0, Universe* universe) = 0;
};