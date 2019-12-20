#pragma once
#include <glm/glm.hpp>

struct CameraUniforms
{
	glm::dmat4 proj, view, c_model;

	// proj * view * c_camera, used for shaders which take it directly
	glm::dmat4 tform;

	// proj * view
	glm::dmat4 proj_view;

	float far_plane;

	glm::dvec3 cam_pos;
};