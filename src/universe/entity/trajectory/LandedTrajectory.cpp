#include "LandedTrajectory.h"
#include <algorithm>



WorldState LandedTrajectory::update(double dt, bool use_bullet)
{
	Universe* uv = osp->universe;
	PlanetarySystem& sys = uv->system;
	WorldState out;

	SystemElement* elem = sys.elements[cached_elem_index];

	double tnow;
	if (use_bullet)
	{
		tnow = sys.bt;
	}
	else
	{
		tnow = sys.t;
	}

	glm::dmat4 rot_matrix = elem->build_rotation_matrix(sys.t0, tnow, false);
	glm::dvec3 body_pos;
	auto st = sys.get_element_state(cached_elem_index, use_bullet);
	body_pos = st.pos;

	glm::dmat4 body_matrix = glm::translate(glm::dmat4(1.0), body_pos);

	glm::dvec3 rel_pos = glm::dvec3(rot_matrix * glm::dvec4(initial_relative_pos, 1.0));
	glm::dvec3 tform_pos = glm::dvec3(body_matrix * glm::dvec4(rel_pos, 1.0));
	glm::dquat tform_rot = glm::dquat(rot_matrix * glm::toMat4(initial_rotation));

	out.pos = tform_pos;
	out.rot = tform_rot;

	// Tangential velocity
	glm::dvec3 tang = elem->get_tangential_speed(rel_pos);

	out.vel = st.vel + tang;

	out.ang_vel = glm::dvec3(0, 0, 0);

	debug_drawer->add_line(out.pos, out.pos + tang, glm::vec3(1.0, 0.0, 1.0));

	return out;
}


void LandedTrajectory::update_element_idx()
{
	auto it = osp->universe->system.name_to_index.find(elem_name);
	logger->check(it != osp->universe->system.name_to_index.end(), "Unable to find element named {}", elem_name);

	cached_elem_index = it->second;

}

LandedTrajectory::~LandedTrajectory()
{
	osp->universe->drop_out_of_event("core:system_update_indices", hndl);
}


LandedTrajectory::LandedTrajectory()
{
	hndl = EventHandler(EventHandlerFnc([](EventArguments& args, const void* ud)
	{
		auto this_ptr = (LandedTrajectory*)ud;
		this_ptr->update_element_idx();
	}), this);
	osp->universe->sign_up_for_event("core:system_update_indices", hndl);
}

void LandedTrajectory::propagate(double dt, const StateVector &mstates, const LightStateVector &lstates,
								 LightCartesianState &our_state)
{
	// TODO
}

void LandedTrajectory::set_element(const std::string &elem)
{
	elem_name = elem;
	update_element_idx();
}
