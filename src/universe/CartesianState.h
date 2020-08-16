#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct CartesianState
{
	glm::dvec3 pos;
	glm::dvec3 vel;

	CartesianState() {}
	CartesianState(glm::dvec3 p, glm::dvec3 v) : pos(p), vel(v) {};
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
		rotation = glm::dquat(1.0, 0.0, 0.0, 0.0);
		angular_velocity = glm::dvec3(0.0, 0.0, 0.0);
	}
};
