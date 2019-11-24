#include "MapCamera.h"
#include "../../util/InputUtil.h"
#include "../../universe/vessel/ReferenceFrame.h"



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

std::pair<glm::dvec3, glm::dvec3> MapCamera::get_camera_pos_dir()
{
	if (frame == nullptr)
	{
		logger->warn("Camera lacks a reference frame!");
		return std::make_pair(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(1.0, 0.0, 0.0));
	}

	glm::dvec3 pos;
	glm::dvec3 dir;

	glm::dvec3 center;
	double center_radius;
	center_radius = 1.0;
	center = frame->get_center();

	if (distance < center_radius * 1.5)
	{
		distance = center_radius * 1.5;
	}

	glm::dvec3 offset = MathUtil::spherical_to_euclidean_r1(circular_coord);
	offset *= center_radius + distance;

	glm::dmat4 rotm = frame->get_rotation_matrix();

	pos = rotm * glm::dvec4(offset, 1.0);

	pos += center;
	dir = glm::normalize(center - pos);

	return std::make_pair(pos, dir);
}

MapCamera::MapCamera()
{
	circular_coord = glm::dvec2(0.0, glm::half_pi<double>());
	distance = 1000000.0;
}


MapCamera::~MapCamera()
{
}
