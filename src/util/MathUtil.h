#pragma once
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

class MathUtil
{
public:

	// Takes and outputs normalized coordinates
	static glm::vec3 cube_to_sphere(glm::vec3 cubic);
	// Takes and outputs normalized coordinates
	static glm::vec3 sphere_to_cube(glm::vec3 spheric);

	static glm::mat4 rotate_from_to(glm::vec3 from, glm::vec3 to);

	// x = azimuth, y = inclination, z = radius
	static glm::vec3 spherical_to_euclidean(glm::vec3 spherical);

	static glm::vec3 spherical_to_euclidean(float azimuth, float inclination, float radius = 1.0f);

	// x = azimuth, y = inclination, z = radius
	static glm::vec3 euclidean_to_spherical(glm::vec3 euclidean);

	// x = azimuth, y = inclination, radius = 1
	static glm::vec3 spherical_to_euclidean_r1(glm::vec2 spherical);

	static glm::vec3 spherical_to_euclidean_r1(float azimuth, float inclination);

	// x = azimuth, y = inclination, r = 1
	static glm::vec2 euclidean_to_spherical_r1(glm::vec3 euclidean);
};

class ProjectionUtil
{
public:

};