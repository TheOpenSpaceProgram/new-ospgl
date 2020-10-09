#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

struct CameraUniforms
{
	glm::dmat4 proj, view, c_model;

	// proj * view * c_camera, used for shaders which take it directly
	glm::dmat4 tform;

	// proj * view
	glm::dmat4 proj_view;

	float far_plane;

	glm::dvec3 cam_pos;

	glm::vec2 screen_size;
	glm::ivec2 iscreen_size;

	GLuint irradiance;
	GLuint specular;
	GLuint brdf;
};