#pragma once

#include "../Trajectory.h"
#include "../../Universe.h"

// Coordinates are relative to the non-rotated planet,
// this means t = 0 without the rotation at epoch applied
class LandedTrajectory : public Trajectory
{
private:

	EventHandler hndl;
	std::string elem_name;
	// Updated in the event handler
	size_t cached_elem_index;

	void update_element_idx();

public:


	void propagate(double dt, const StateVector& mstates,
						   const LightStateVector& lstates, LightCartesianState& our_state) override;

	WorldState update(double dt, bool use_bullet) override;

	glm::dvec3 initial_relative_pos;
	glm::dquat initial_rotation;

	void set_element(const std::string& elem);

	LandedTrajectory();
	~LandedTrajectory();

};

