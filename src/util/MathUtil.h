#pragma once
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

class MathUtil
{
public:

	// Takes and outputs normalized coordinates
	static glm::dvec3 cube_to_sphere(glm::dvec3 cubic);
	// Takes and outputs normalized coordinates
	static glm::dvec3 sphere_to_cube(glm::dvec3 spheric);

	static glm::mat4 rotate_from_to(glm::vec3 from, glm::vec3 to);

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
};

class ProjectionUtil
{
public:

};