#include "LandedTrajectory.h"



LandedTrajectory::LandedTrajectory(std::string str)
{
	in_planet = str;
}


LandedTrajectory::~LandedTrajectory()
{
}

WorldState LandedTrajectory::get_state(double t_now)
{
	return WorldState();
}

void LandedTrajectory::update(double dt)
{
	// We do nothing here, we are a simple function of time
}

void LandedTrajectory::start(WorldState s0, double t0, Universe * universe)
{
	elem_index = universe->system.get_element_index_from_name(in_planet);
	// Calculate relative position and orientation

}
