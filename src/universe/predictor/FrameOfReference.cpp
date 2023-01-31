#include "FrameOfReference.h"
#include "universe/Universe.h"
#include "universe/PlanetarySystem.h"

glm::dvec3 FrameOfReference::get_rel_pos(glm::dvec3 abs_pos, glm::dvec3 body_pos, double body_rot)
{
	if(rotating)
	{
		return glm::dvec3(0, 0, 0);
	}
	else
	{
		return abs_pos - body_pos - offset;
	}
}


std::pair<glm::dvec3, glm::dvec3>
FrameOfReference::get_rel_pos_vel(glm::dvec3 abs_pos, glm::dvec3 abs_vel, glm::dvec3 body_pos, glm::dvec3 body_vel,
								  double body_rot, double body_rotvel)
{
	auto pos = get_rel_pos(abs_pos, body_pos, body_rot);
	glm::dvec3 vel;
	if(rotating)
	{
		vel = glm::dvec3(0, 0, 0);
	}
	else
	{
		vel = abs_vel - body_vel;
	}

	return std::make_pair(pos, vel);
}

glm::dmat4 FrameOfReference::get_tform_matrix()
{
	glm::dvec3 pos = osp->universe->system.states_now[center_id].pos;
	return glm::translate(glm::dmat4(1.0), pos);
}

FrameOfReference::FrameOfReference()
{
	offset = glm::dvec3(0, 0 ,0);
	rotating = 0;
	center_id = 1;

}
