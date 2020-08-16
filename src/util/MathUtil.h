#pragma once
#include <cmath>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tuple>

class MathUtil
{
public:

	// Takes and outputs normalized coordinates
	static glm::dvec3 cube_to_sphere(glm::dvec3 cubic);
	// Takes and outputs normalized coordinates
	static glm::dvec3 sphere_to_cube(glm::dvec3 spheric);

	static glm::dmat4 rotate_from_to(glm::dvec3 from, glm::dvec3 to);

	// x = azimuth, y = inclination, z = radius
	static glm::dvec3 spherical_to_euclidean(glm::dvec3 spherical);

	static glm::dvec3 spherical_to_euclidean(double azimuth, double inclination, double radius = 1.0f);

	// x = azimuth, y = inclination, z = radius
	static glm::dvec3 euclidean_to_spherical(glm::dvec3 euclidean);

	// x = azimuth, y = inclination, radius = 1
	static glm::dvec3 spherical_to_euclidean_r1(glm::dvec2 spherical);

	static glm::dvec3 spherical_to_euclidean_r1(double azimuth, double inclination);

	// x = azimuth, y = inclination, r = 1
	static glm::dvec2 euclidean_to_spherical_r1(glm::dvec3 euclidean);

	static glm::dquat quat_look_at(glm::dvec3 from, glm::dvec3 to,
		glm::dvec3 up = glm::dvec3(0, 1, 0), glm::dvec3 alt_up = glm::dvec3(0, 0, 1));

	static double distance_to_line(glm::dvec3 la, glm::dvec3 lb, glm::dvec3 from);

	// Similar to GL_REPEAT
	static int int_repeat(int v, int max);
	// Similar to GL_CLAMP
	static int int_clamp(int v, int max);

	// Generates the start and end (in that order) of a ray given the properties
	// mouse_pos must be in normalized device coordinates [-1 -> 1], y inverted
	static std::pair<glm::dvec3, glm::dvec3> screen_raycast(glm::dvec2 mouse_pos, glm::dmat4 inverse_camera, 
			double distance);

	// The bool indicates wether the point is in front of the camera or not
	// Coordinates are gl clip space, use another helper function to use them on real GUI
	static std::pair<glm::dvec2, bool> world_to_clip(glm::dmat4 proj_view, glm::dvec3 wpoint);

	// Viewport is (x0, y0, w, h), as used everywhere
	// Returns the coordinates to be fed into NanoVG
	static glm::vec2 clip_to_screen(glm::dvec2 clip_pos, glm::vec4 viewport);
};

class ProjectionUtil
{
public:

};
