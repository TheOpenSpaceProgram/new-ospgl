#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// Doesn't include mass, used for vessels and other non-attracting bodies
struct LightCartesianState
{
	glm::dvec3 pos;
	glm::dvec3 vel;
};

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
	glm::dvec3 pos;
	glm::dvec3 vel;
	glm::dquat rot;
	glm::dvec3 ang_vel;

	WorldState()
	{
		// Sane defaults
		pos = glm::dvec3(0.0, 0.0, 0.0);
		vel = glm::dvec3(0.0, 0.0, 0.0);
		rot = glm::dquat(1.0, 0.0, 0.0, 0.0);
		ang_vel = glm::dvec3(0.0, 0.0, 0.0);
	}
};
