#pragma once

#include "../Trajectory.h"
#include "../../Universe.h"

// Coordinates are relative to the non-rotated planet,
// this means t = 0 without the rotation at epoch applied
class LandedTrajectory : public Trajectory
{
private:

	std::string elem_name;
	glm::dvec3 initial_relative_pos;
	glm::dquat initial_rotation;
	Universe* universe;

public:


	virtual WorldState get_state(double t0, double t, bool use_bullet = false) override;
	void set_parameters(std::string body_name, glm::dvec3 rel_pos, glm::dquat rel_rot);

	LandedTrajectory();
	~LandedTrajectory();


};

