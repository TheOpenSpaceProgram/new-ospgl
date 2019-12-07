#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tuple>

class SimpleCamera
{
private:

public:

	// In degrees
	double fov;

	glm::dvec3 pos;
	glm::dvec3 fw;

	void update(double dt);

	std::pair<glm::dvec3, glm::dvec3> get_camera_pos_dir();

	glm::dmat4 get_proj_view(int w, int h);
	glm::dmat4 get_cmodel();


	SimpleCamera();
	~SimpleCamera();
};
