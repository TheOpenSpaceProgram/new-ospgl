#include "ReferenceFrame.h"

glm::dvec3 ReferenceFrame::get_velocity() const
{
	if (mode == ALIGNED_BARYCENTRIC)
	{
		glm::dvec3 c = center.get_velocity_now();
		glm::dvec3 s = center2->get_velocity_now();

		double c_mass = center.get_mass();
		double s_mass = center2->get_mass();

		glm::dvec3 average = (c * c_mass + s * s_mass) / (c_mass + s_mass);
		return average;
	}
	else
	{
		return center.get_velocity_now();
	}
	
}

glm::dvec3 ReferenceFrame::get_center() const
{
	if (mode == ALIGNED_BARYCENTRIC)
	{
		glm::dvec3 c = center.get_position_now();
		glm::dvec3 s = center2->get_position_now();

		double c_mass = center.get_mass();
		double s_mass = center2->get_mass();

		glm::dvec3 average = (c * c_mass + s * s_mass) / (c_mass + s_mass);
		return average;
	}
	else
	{
		return center.get_position_now();
	}
}

glm::dvec3 ReferenceFrame::get_x_axis(double t) const
{
	// isinf(t) just checks for now time, a bit hacky but works

	glm::dvec3 x = glm::dvec3(1.0, 0.0, 0.0);

	if (!std::isinf(t))
	{
		logger->warn("Implement get_x at given time");
		return x;
	}

	if (mode == ReferenceFrame::INERTIAL)
	{
		if (center.is_vessel)
		{
			return x;
		}
		else
		{
			SystemElement* celem = center.get_element();
			if (celem->is_barycenter)
			{
				return x;
			}
			else
			{
				return celem->as_body->build_rotation_matrix(0, false) * glm::dvec4(x, 1.0);
			}
			
		}
		
	}
	else if (mode == ReferenceFrame::ROTATING)
	{
		if (center.is_vessel)
		{
			return center.get_vessel()->get_forward();
		}

		SystemElement* celem = center.get_element();
		if (celem->is_barycenter)
		{
			if (std::isinf(t))
			{
				glm::dvec3 primary = center.system->states_now[celem->barycenter_primary->index + 1].pos;
				glm::dvec3 secondary = center.system->states_now[celem->barycenter_secondary->index + 1].pos;

				return glm::normalize(secondary - primary);
			}

			return x;
		}
		else
		{
			if (std::isinf(t))
			{
				return celem->as_body->build_rotation_matrix(center.system->t) * glm::dvec4(x, 1.0);

			}
			
			return celem->as_body->build_rotation_matrix(t) * glm::dvec4(x, 1.0);
		}
		
	}
	else if (mode == ReferenceFrame::SURFACE)
	{
		logger->warn("TODO: Implement surface get_x");
		return x;
	}
	else if (mode == ReferenceFrame::PARENT_ALIGNED)
	{
		if (center.is_vessel)
		{
			logger->warn("Parent aligned cannot be used in vessels!");
			return x;
		}

		if (std::isinf(t))
		{
			glm::dvec3 cpos = center.get_position_now();
			glm::dvec3 ppos;
			if (center.get_element()->parent == nullptr)
			{
				ppos = glm::dvec3(0.0, 0.0, 0.0);
			}
			else
			{
				ppos = center.system->states_now[center.get_element()->parent->index + 1].pos;
			}

			return glm::normalize(ppos - cpos);
		}

		return x;
	}
	else if (mode == ReferenceFrame::ALIGNED)
	{
		if (!center2.has_value())
		{
			logger->warn("Tried to use aligned reference frame without a secondary body!");
			return x;
		}

		if (std::isinf(t))
		{
			glm::dvec3 spos = center2->get_position_now();
			glm::dvec3 ppos = center.get_position_now();

			glm::dvec3 relp = spos - ppos;

			return glm::normalize(relp);
		}
	}
	else if (mode == ReferenceFrame::ALIGNED_BARYCENTRIC)
	{
		glm::dvec3 center = get_center();
		glm::dvec3 s_pos = center2->get_position_now();

		return glm::normalize(s_pos - center);
	}

	return x;
}

glm::dvec3 ReferenceFrame::get_y_axis(double t) const
{
	// isinf(t) just checks for now time, a bit hacky but works
	glm::dvec3 y = glm::dvec3(0.0, 1.0, 0.0);

	if (!std::isinf(t))
	{
		logger->warn("Implement get_y at given time");
		return y;
	}

	if (mode == ReferenceFrame::INERTIAL)
	{
		if (center.is_vessel)
		{
			return y;
		}
		else
		{
			SystemElement* celem = center.get_element();
			if (celem->is_barycenter)
			{
				return y;
			}
			else
			{
				return celem->as_body->build_rotation_matrix(0, false) * glm::dvec4(y, 1.0);
			}

		}

	}
	else if (mode == ReferenceFrame::ROTATING)
	{
		if (center.is_vessel)
		{
			return center.get_vessel()->get_up();
		}

		SystemElement* celem = center.get_element();
		if (celem->is_barycenter)
		{
			if (std::isinf(t))
			{
				return glm::normalize(celem->barycenter_secondary->orbit.to_orbit_at(center.system->t).get_plane_normal());
			}
			return y;
		}
		else
		{
			if (std::isinf(t))
			{
				return celem->as_body->build_rotation_matrix(center.system->t) * glm::dvec4(y, 1.0);

			}

			return celem->as_body->build_rotation_matrix(t) * glm::dvec4(y, 1.0);
		}

	}
	else if (mode == ReferenceFrame::SURFACE)
	{
		logger->warn("TODO: Implement surface get_x");
		return y;
	}
	else if (mode == ReferenceFrame::PARENT_ALIGNED)
	{
		if (center.is_vessel)
		{
			logger->warn("Parent aligned cannot be used in vessels!");
			return y;
		}

		if (std::isinf(t))
		{
			// Orbit plane normal

			return glm::normalize(center.get_element()->orbit.to_orbit_at(center.system->t).get_plane_normal());
		}

		return y;
	}
	else if (mode == ReferenceFrame::ALIGNED)
	{
		if (std::isinf(t))
		{
			glm::dvec3 svel = center2->get_velocity_now();
			glm::dvec3 pvel = center.get_velocity_now();

			glm::dvec3 spos = center2->get_position_now();
			glm::dvec3 ppos = center.get_position_now();

			glm::dvec3 relp = spos - ppos;
			glm::dvec3 relv = svel - pvel;


			return -glm::normalize(glm::cross(glm::normalize(relv), glm::normalize(relp)));
		}

	}
	else if (mode == ReferenceFrame::ALIGNED_BARYCENTRIC)
	{
		glm::dvec3 center_vel = get_velocity();
		glm::dvec3 s_vel = center2->get_velocity_now();
		glm::dvec3 c_vel = center.get_velocity_now();

		glm::dvec3 s_vel_rel = s_vel;
		glm::dvec3 c_vel_rel = c_vel;

		return glm::normalize(glm::cross(s_vel_rel, c_vel_rel));
	}

	return y;
}

glm::dvec3 ReferenceFrame::get_z_axis(double t) const
{
	return glm::cross(get_x_axis(t), get_y_axis(t));
}

glm::dmat4 ReferenceFrame::get_rotation_matrix() const
{
	return glm::dmat3(get_x_axis(), get_y_axis(), get_z_axis());
}

#include "../../util/DebugDrawer.h"



void ReferenceFrame::draw_debug_axes() const
{
	glm::dvec3 x = get_x_axis();
	glm::dvec3 y = get_y_axis();
	glm::dvec3 z = get_z_axis();

	double l = 1e9;
	
	glm::dvec3 c = get_center();

	debug_drawer->add_arrow(c, c + x * l, glm::vec3(1.0, 0.0, 0.0));
	debug_drawer->add_arrow(c, c + y * l, glm::vec3(0.0, 1.0, 0.0));
	debug_drawer->add_arrow(c, c + z * l, glm::vec3(0.0, 0.0, 1.0));

}
