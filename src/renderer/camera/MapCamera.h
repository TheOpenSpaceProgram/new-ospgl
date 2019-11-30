#pragma once
#include <glm/glm.hpp>
#include <vector>

struct ReferenceFrame;

class MapCamera
{
private:

	double scroll_vel;

public:

	ReferenceFrame* frame;

	// In meters FROM SURFACE (so zoom is easy to implement)

	double distance;
	glm::dvec2 circular_coord;

	// Handles input
	void update(double dt);

	// Relative to system, not to center planet
	std::pair<glm::dvec3, glm::dvec3> get_camera_pos_dir();

	MapCamera();
	~MapCamera();
};

