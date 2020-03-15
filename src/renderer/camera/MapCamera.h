#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "../../util/Logger.h"
#include "../../util/MathUtil.h"
#include "../../universe/SystemPointer.h"
#include <glm/gtc/matrix_transform.hpp>

#include "CameraUniforms.h"
#include "Camera.h"

// Allows locking the camera on anything, but only
// on intertial reference frames
class MapCamera : public Camera
{
private:

	double scroll_vel;
	glm::dmat4 get_proj_view(int w, int h);
	glm::dmat4 get_cmodel();

public:

	// In degrees
	double fov;

	SystemPointer center_ptr;
	// In meters FROM SURFACE (so zoom is easy to implement)

	double distance;
	glm::dvec2 circular_coord;

	// Handles input
	void update(double dt);

	// Relative to system, not to center planet
	std::pair<glm::dvec3, glm::dvec3> get_camera_pos_dir();

	virtual CameraUniforms get_camera_uniforms(int w, int h) override;

	MapCamera(SystemPointer ptr);
	~MapCamera();
};

