#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>

struct ShadowCamera
{
	glm::dmat4 c_camera;
	glm::dmat4 proj;
	glm::dmat4 view;
	GLuint fbuffer;
	int size;
	float far_plane;

	glm::dmat4 tform;
	glm::dvec3 cam_pos;
};
