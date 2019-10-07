#include "MathUtil.h"

// http://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html
glm::vec3 MathUtil::cube_to_sphere(glm::vec3 cubic)
{
	glm::vec3 out;

	float xsq = cubic.x * cubic.x;
	float ysq = cubic.y * cubic.y;
	float zsq = cubic.z * cubic.z;

	out.x = cubic.x * sqrtf(1.0f - (ysq / 2.0f) - (zsq / 2.0f) + ((ysq * zsq) / 3.0f));
	out.y = cubic.y * sqrtf(1.0f - (xsq / 2.0f) - (zsq / 2.0f) + ((xsq * zsq) / 3.0f));
	out.z = cubic.z * sqrtf(1.0f - (xsq / 2.0f) - (ysq / 2.0f) + ((xsq * ysq) / 3.0f));

	return out;
}


// https://stackoverflow.com/questions/2656899/mapping-a-sphere-to-a-cube#2698997
// Trying to calculate the inverse of cube_to_sphere was quite hard
glm::vec3 to_cube(glm::vec3 vec)
{
	float xx2 = vec.x * vec.x * 2.0f;
	float yy2 = vec.y * vec.y * 2.0f;

	glm::vec2 v = glm::vec2(xx2 - yy2, yy2 - xx2);
	float ii = v.y - 3.0f; ii *= ii;

	float isqrt = -sqrtf(ii - 12.0f * xx2) + 3.0f;
	v = glm::sqrt(v + isqrt);
	v *= 0.70710676908493042f;

	return glm::sign(vec) * glm::vec3(v, 1.0f);
}

glm::vec3 MathUtil::sphere_to_cube(glm::vec3 spheric)
{
	glm::vec3 f = glm::abs(spheric);

	bool a = f.y >= f.x && f.y >= f.z;
	bool b = f.x >= f.z;

	if (a)
	{
		glm::vec3 v = to_cube(glm::vec3(spheric.x, spheric.z, spheric.y));
		return glm::vec3(v.x, v.z, v.y);
	}
	else if (b)
	{
		glm::vec3 v = to_cube(glm::vec3(spheric.y, spheric.z, spheric.x));
		return glm::vec3(v.z, v.x, v.y);
	}
	else
	{
		return to_cube(spheric);
	}
}

glm::mat4 MathUtil::rotate_from_to(glm::vec3 from, glm::vec3 to)
{
	glm::vec3 axis = glm::normalize(glm::cross(from, to));
	float dot = glm::dot(from, to);
	float angle = glm::acos(dot / (from.length() * to.length()));

	return glm::rotate(angle, axis);
}

glm::vec3 MathUtil::spherical_to_euclidean(glm::vec3 spherical)
{
	return spherical_to_euclidean(spherical.x, spherical.y, spherical.z);
}

glm::vec3 MathUtil::spherical_to_euclidean(float azimuth, float inclination, float radius)
{
	glm::vec3 out;

	out.x = sin(inclination) * cos(azimuth);
	out.y = cos(inclination);
	out.z = sin(inclination) * sin(azimuth);

	return radius * out;
}

glm::vec3 MathUtil::euclidean_to_spherical(glm::vec3 eu)
{
	glm::vec3 out;

	out.z = glm::sqrt(eu.x * eu.x + eu.y * eu.y + eu.z * eu.z);
	out.x = glm::atan(eu.z / eu.x);
	out.y = glm::acos(eu.y / out.z);

	return out;
}

glm::vec3 MathUtil::spherical_to_euclidean_r1(glm::vec2 spherical)
{
	return spherical_to_euclidean_r1(spherical.x, spherical.y);
}

glm::vec3 MathUtil::spherical_to_euclidean_r1(float azimuth, float inclination)
{
	glm::vec3 out;

	out.x = sin(inclination) * cos(azimuth);
	out.y = cos(inclination);
	out.z = sin(inclination) * sin(azimuth);

	return out;
}

glm::vec2 MathUtil::euclidean_to_spherical_r1(glm::vec3 eu)
{
	glm::vec3 out;

	out.x = glm::atan(eu.z / eu.x);
	out.y = glm::acos(eu.y);

	return out;
}
