#include "SimpleCamera.h"



void SimpleCamera::update(double dt)
{

}

std::pair<glm::dvec3, glm::dvec3> SimpleCamera::get_camera_pos_dir()
{
	return std::make_pair(pos, fw);
}

glm::dmat4 SimpleCamera::get_proj_view(int width, int height)
{
	auto[camera_pos, camera_dir] = get_camera_pos_dir();
	// ~1 light year
	float far_plane = 1e16f;


	glm::dmat4 proj = glm::perspective(glm::radians(fov), (double)width / (double)height, 0.1, (double)far_plane);
	glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), camera_dir, glm::dvec3(0.0, 1.0, 0.0));
	glm::dmat4 proj_view = proj * view;

	return proj_view;
}

glm::dmat4 SimpleCamera::get_cmodel()
{
	return glm::translate(glm::dmat4(1.0), -get_camera_pos_dir().first);
}

SimpleCamera::SimpleCamera()
{
	fov = 60.0;
	pos = glm::dvec3(0.0, 0.0, 0.0);
	fw = glm::dvec3(1.0, 0.0, 0.0);
}


SimpleCamera::~SimpleCamera()
{
}
