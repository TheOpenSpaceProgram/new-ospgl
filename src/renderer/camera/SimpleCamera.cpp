#include "SimpleCamera.h"



void SimpleCamera::update(double dt)
{

}

std::pair<glm::dvec3, glm::dvec3> SimpleCamera::get_camera_pos_dir()
{
	return std::make_pair(pos, fw);
}

CameraUniforms SimpleCamera::get_camera_uniforms(int w, int h)
{
	CameraUniforms out;

	auto[camera_pos, camera_dir] = get_camera_pos_dir();
	// ~1 light year
	float far_plane = 1e16f;


	glm::dmat4 proj = glm::perspective(glm::radians(fov), (double)w / (double)h, 0.1, (double)far_plane);
	glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), camera_dir, glm::dvec3(0.0, 1.0, 0.0));
	glm::dmat4 proj_view = proj * view;

	out.proj = proj;
	out.view = view;
	out.proj_view = proj_view;
	out.c_model = glm::translate(glm::dmat4(1.0), -camera_pos);
	out.tform = proj * view * out.c_model;
	out.far_plane = 10e16f;
	out.cam_pos = camera_pos;

	return out;
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
