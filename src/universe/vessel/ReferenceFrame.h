#pragma once

#include <glm/gtx/quaternion.hpp>
#include "../SystemPointer.h"

class PlanetarySystem;

struct ReferenceFrame
{
	SystemPointer center;
	bool rotating;

	// All transformation at done with the system's get now
	// functions
	glm::dvec3 relative_velocity(glm::dvec3 vel) const;

	glm::dquat forward_rotation() const;

	ReferenceFrame(SystemPointer c) : center(c)
	{
	}
};