#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "../../universe/SystemElement.h"

class MapCamera
{
private:

	double scroll_vel;

public:

	// -1 for sun
	// -2 for vessel
	int focus_index;

	// In meters FROM SURFACE (so zoom is easy to implement)

	double distance;
	glm::dvec2 circular_coord;

	// Handles input
	void update(double dt);

	// Relative to system, not to center planet
	std::pair<glm::dvec3, glm::dvec3> get_camera_pos_dir(double t, glm::dvec3 vessel_pos,
		double star_radius, std::vector<CartesianState>& render_states,
		std::vector<SystemElement>& bodies);

	MapCamera();
	~MapCamera();
};

