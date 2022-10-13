#include "LandedTrajectory.h"
#include <algorithm>



LandedTrajectory::LandedTrajectory()
{
	changed = true;
	elem_name = "";	//< To crash on bad initialiations
}


LandedTrajectory::~LandedTrajectory()
{
}

WorldState LandedTrajectory::get_state(double _unused, double _unused2, bool use_bullet)
{
	if (changed)
	{
		elem_index = get_universe()->system.get_element_index_from_name(elem_name);
		changed = false;
	}


	WorldState out;

	SystemElement* elem = get_universe()->system.elements[elem_index];

	double tnow;
	if (use_bullet)
	{
		tnow = get_universe()->system.bt;
	}
	else
	{
		tnow = get_universe()->system.t;
	}

	glm::dmat4 rot_matrix = elem->build_rotation_matrix(get_universe()->system.t0, tnow, false);
	glm::dvec3 body_pos;
	if (use_bullet)
	{
		body_pos = get_universe()->system.bullet_states[elem_index].pos;
	}
	else
	{
		body_pos = get_universe()->system.states_now[elem_index].pos;
	}

	glm::dmat4 body_matrix = glm::translate(glm::dmat4(1.0), body_pos);

	glm::dvec3 rel_pos = glm::dvec3(rot_matrix * glm::dvec4(initial_relative_pos, 1.0));
	glm::dvec3 tform_pos = glm::dvec3(body_matrix * glm::dvec4(rel_pos, 1.0));
	glm::dquat tform_rot = glm::dquat(rot_matrix * glm::toMat4(initial_rotation));

	out.cartesian.pos = tform_pos;
	out.rotation = tform_rot;

	// Tangential velocity
	glm::dvec3 tang = elem->get_tangential_speed(rel_pos);

	out.cartesian.vel = get_universe()->system.bullet_states[elem_index].vel + tang;

	out.angular_velocity = glm::dvec3(0, 0, 0);

	debug_drawer->add_line(out.cartesian.pos, out.cartesian.pos + tang, glm::vec3(1.0, 0.0, 1.0));

	return out;
}


void LandedTrajectory::set_parameters(std::string in_body, glm::dvec3 rel_pos, glm::dquat rel_rot)
{
	this->elem_name = in_body;
	this->initial_relative_pos = rel_pos;
	this->initial_rotation = rel_rot;
	changed = true;
}
