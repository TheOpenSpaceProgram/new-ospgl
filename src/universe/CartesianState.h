#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct CartesianState
{
	glm::dvec3 pos;
	glm::dvec3 vel;
	// Avoids vector lookup, not needed for non-attracting bodies
	double mass;

	CartesianState() {}
	//CartesianState(glm::dvec3 p, glm::dvec3 v) : pos(p), vel(v), mass(0.0) {};
	CartesianState(glm::dvec3 p, glm::dvec3 v, double m) : pos(p), vel(v), mass(m) {};
};

// Also includes orientation
struct WorldState
{
	CartesianState cartesian;
	glm::dquat rotation;
	glm::dvec3 angular_velocity;

	WorldState()
	{
		// Sane defaults
		cartesian.pos = glm::dvec3(0.0, 0.0, 0.0);
		cartesian.vel = glm::dvec3(0.0, 0.0, 0.0);
		cartesian.mass = 0.0;
		rotation = glm::dquat(1.0, 0.0, 0.0, 0.0);
		angular_velocity = glm::dvec3(0.0, 0.0, 0.0);
	}
};
