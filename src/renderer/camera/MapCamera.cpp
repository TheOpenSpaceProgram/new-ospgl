#include "MapCamera.h"
#include "../../util/InputUtil.h"


void MapCamera::update(double dt)
{
	scroll_vel -= input->mouse_scroll_delta;
	distance += scroll_vel * distance * dt;

	double prev = scroll_vel;
	scroll_vel -= scroll_vel * dt * 8.0;
	if (prev * scroll_vel < 0.0 || glm::abs(scroll_vel) <= 0.05)
	{
		scroll_vel = 0.0;
	}

	if (glfwGetMouseButton(input->window, 1) == GLFW_PRESS)
	{
		circular_coord.x += input->mouse_delta.x * 0.004;
		circular_coord.y -= input->mouse_delta.y * 0.004;

		circular_coord.y = glm::clamp(circular_coord.y, 0.05, glm::pi<double>() * 0.95);
	}
}

std::pair<glm::dvec3, glm::dvec3> MapCamera::get_camera_pos_dir(
	double t, glm::dvec3 vessel_pos, double star_radius, std::vector<CartesianState>& states_now,
	std::vector<SystemElement>& bodies)
{
	glm::dvec3 pos;
	glm::dvec3 dir;

	glm::dvec3 center;
	double center_radius;
	if (focus_index == -1)
	{
		center = vessel_pos;
		center_radius = 1.0;
	}
	else
	{
		center = states_now[focus_index].pos;
		if (focus_index == 0)
		{
			center_radius = star_radius;
		}
		else
		{
			if (bodies[focus_index - 1].is_barycenter)
			{
				center_radius = 1.0;
			}
			else
			{
				center_radius = bodies[focus_index - 1].as_body->config.radius;
			}
		}
		
	}

	if (distance < center_radius * 1.5)
	{
		distance = center_radius * 1.5;
	}

	glm::dvec3 offset = MathUtil::spherical_to_euclidean_r1(circular_coord);
	offset *= center_radius + distance;

	pos = center + offset;
	dir = glm::normalize(center - pos);

	return std::make_pair(pos, dir);
}

MapCamera::MapCamera()
{
	focus_index = -1;
	circular_coord = glm::dvec2(0.0, glm::half_pi<double>());
	distance = 1000000.0;
}


MapCamera::~MapCamera()
{
}
