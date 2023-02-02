#pragma once
#include "../CartesianState.h"
#include "../UniverseDefinitions.h"
#include <vector>

class Universe;

// Trajectories are used to implement behaviour other than n-body physics
// in prediction and propagation. A good example are landed vessels, which are
// fixed to a point in a planet
// Trajectories may also be used during real-time simulation in bullet
// Iedally, implement them in C++ for performance as these are called a LOT
class Trajectory
{
public:

	// Called once per timestep, with values of the system elements, and light state vectors, after n-body simulation
	// You are included in lstates, but you must modify the passed LightState
	// t0 is the system start time, to allow massive times without floating point errors for planet rotations
	virtual void propagate(double dt, const StateVector& mstates,
						   const LightStateVector& lstates, LightCartesianState& our_state) = 0;

	// Called in real-time, so that the trajectory can work for visually positioning the object
	// and for positioning it in the physics engine (if use_bullet is true)
	virtual WorldState update(double dt, bool use_bullet) = 0;

};
