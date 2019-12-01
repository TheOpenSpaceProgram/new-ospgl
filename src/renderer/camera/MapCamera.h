#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "../../util/Logger.h"
#include "../../util/MathUtil.h"
#include "../../universe/SystemPointer.h"
#include <glm/gtc/matrix_transform.hpp>

// Allows locking the camera on anything, but only
// on intertial reference frames
class MapCamera
{
private:

	double scroll_vel;

public:

	SystemPointer center_ptr;
	// In meters FROM SURFACE (so zoom is easy to implement)

	double distance;
	glm::dvec2 circular_coord;

	// Handles input
	void update(double dt);

	// Relative to system, not to center planet
	std::pair<glm::dvec3, glm::dvec3> get_camera_pos_dir();

	MapCamera(SystemPointer ptr);
	~MapCamera();
};

