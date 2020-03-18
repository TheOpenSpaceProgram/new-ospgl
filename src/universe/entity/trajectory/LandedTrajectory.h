#pragma once

#include "../Trajectory.h"
#include "../../Universe.h"

class LandedTrajectory : public Trajectory
{
private:

	glm::dvec3 initial_relative_pos;
	glm::dquat initial_rotation;

	std::string in_planet;
	size_t elem_index;

public:

	virtual WorldState get_state(double t_now) override;
	virtual void update(double dt) override;
	virtual void start(WorldState s0, double t0, Universe* universe) override;

	LandedTrajectory(std::string in_planet);
	~LandedTrajectory();


};

