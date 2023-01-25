#include "LandedTrajectory.h"
#include <algorithm>



LandedTrajectory::LandedTrajectory()
{
	elem_name = "";	//< To crash on bad initialiations
}


LandedTrajectory::~LandedTrajectory()
{
}

WorldState LandedTrajectory::get_state(double _unused, double _unused2, bool use_bullet)
{

	WorldState out;

	size_t idx = get_universe()->system.name_to_index[elem_name];
	SystemElement* elem = get_universe()->system.elements[idx];

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
	auto st = get_universe()->system.get_element_state(idx, use_bullet);
	body_pos = st.pos;

	glm::dmat4 body_matrix = glm::translate(glm::dmat4(1.0), body_pos);

	glm::dvec3 rel_pos = glm::dvec3(rot_matrix * glm::dvec4(initial_relative_pos, 1.0));
	glm::dvec3 tform_pos = glm::dvec3(body_matrix * glm::dvec4(rel_pos, 1.0));
	glm::dquat tform_rot = glm::dquat(rot_matrix * glm::toMat4(initial_rotation));

	out.cartesian.pos = tform_pos;
	out.rotation = tform_rot;

	// Tangential velocity
	glm::dvec3 tang = elem->get_tangential_speed(rel_pos);

	out.cartesian.vel = st.vel + tang;

	out.angular_velocity = glm::dvec3(0, 0, 0);

	debug_drawer->add_line(out.cartesian.pos, out.cartesian.pos + tang, glm::vec3(1.0, 0.0, 1.0));

	return out;
}


void LandedTrajectory::set_parameters(std::string in_body, glm::dvec3 rel_pos, glm::dquat rel_rot)
{
	this->elem_name = in_body;
	this->initial_relative_pos = rel_pos;
	this->initial_rotation = rel_rot;
}
