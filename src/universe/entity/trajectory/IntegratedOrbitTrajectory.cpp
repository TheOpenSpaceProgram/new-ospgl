#include "IntegratedOrbitTrajectory.h"



IntegratedOrbitTrajectory::IntegratedOrbitTrajectory()
{

}


IntegratedOrbitTrajectory::~IntegratedOrbitTrajectory()
{
}

WorldState IntegratedOrbitTrajectory::get_state(double t_now, bool use_bullet)
{
	logger->check(this->t_now = t_now, "IntegratedOrbitTrajectory is out of sync");

	return current;
}

void IntegratedOrbitTrajectory::update(double dt)
{
	t_now += dt;
	// Integrate (The propagator has been prepared)
	universe->system.propagator->propagate(&current.cartesian);

	double l = glm::length(current.angular_velocity);
	current.rotation *= glm::angleAxis(l * dt, current.angular_velocity / l);
}

void IntegratedOrbitTrajectory::start(WorldState s0, double t0, Universe* universe)
{
	current = s0;
	t_now = t0;
	this->universe = universe;
	this->propagator = universe->system.propagator;
}
