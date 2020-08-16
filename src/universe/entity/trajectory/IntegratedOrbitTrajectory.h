#pragma once

#include "../Trajectory.h"
#include "../../Universe.h"
#include "../../propagator/SystemPropagator.h"

// We directly use the orbit propagator in the
// PlanetarySystem. This allows all vessels to share
// the system position computations
class IntegratedOrbitTrajectory : public Trajectory
{
private:

	SystemPropagator* propagator;
	WorldState current;
	double t_now;
	Universe* universe;

public:
	IntegratedOrbitTrajectory();
	~IntegratedOrbitTrajectory();

	// Inherited via Trajectory
	virtual WorldState get_state(double t0, double t, bool use_bullet = false) override;
	void update(double dt);
	void start(WorldState s0, double t0, Universe* universe);
};

