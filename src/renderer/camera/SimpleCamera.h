#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tuple>
#include "CameraUniforms.h"


class SimpleCamera
{
private:
	glm::dmat4 get_proj_view(int w, int h);
	glm::dmat4 get_cmodel();

public:

	// In degrees
	double fov;

	glm::dvec3 pos;
	glm::dvec3 fw;

	void update(double dt);

	// Relative to system, not to center planet
	std::pair<glm::dvec3, glm::dvec3> get_camera_pos_dir();

	CameraUniforms get_camera_uniforms(int w, int h);



	SimpleCamera();
	~SimpleCamera();
};
