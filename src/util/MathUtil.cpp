#include "MathUtil.h"
#include "Logger.h"

// http://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html
glm::dvec3 MathUtil::cube_to_sphere(glm::dvec3 cubic)
{
	glm::dvec3 out;

	double xsq = cubic.x * cubic.x;
	double ysq = cubic.y * cubic.y;
	double zsq = cubic.z * cubic.z;

	out.x = cubic.x * sqrt(1.0 - (ysq / 2.0) - (zsq / 2.0) + ((ysq * zsq) / 3.0));
	out.y = cubic.y * sqrt(1.0 - (xsq / 2.0) - (zsq / 2.0) + ((xsq * zsq) / 3.0));
	out.z = cubic.z * sqrt(1.0 - (xsq / 2.0) - (ysq / 2.0) + ((xsq * ysq) / 3.0));

	return out;
}


// https://stackoverflow.com/questions/2656899/mapping-a-sphere-to-a-cube#2698997
// Trying to calculate the inverse of cube_to_sphere was quite hard
glm::dvec3 to_cube(glm::dvec3 vec)
{
	double xx2 = vec.x * vec.x * 2.0;
	double yy2 = vec.y * vec.y * 2.0;

	glm::dvec2 v = glm::dvec2(xx2 - yy2, yy2 - xx2);
	double ii = v.y - 3.0; ii *= ii;

	double isqrt = -sqrt(ii - 12.0 * xx2) + 3.0;
	v = glm::sqrt(v + isqrt);
	v *= 0.70710676908493042;

	return glm::sign(vec) * glm::dvec3(v, 1.0f);
}

glm::dvec3 MathUtil::sphere_to_cube(glm::dvec3 spheric)
{
	glm::dvec3 f = glm::abs(spheric);

	bool a = f.y >= f.x && f.y >= f.z;
	bool b = f.x >= f.z;

	if (a)
	{
		glm::dvec3 v = to_cube(glm::dvec3(spheric.x, spheric.z, spheric.y));
		return glm::dvec3(v.x, v.z, v.y);
	}
	else if (b)
	{
		glm::dvec3 v = to_cube(glm::dvec3(spheric.y, spheric.z, spheric.x));
		return glm::dvec3(v.z, v.x, v.y);
	}
	else
	{
		return to_cube(spheric);
	}
}

glm::dmat4 MathUtil::rotate_from_to(glm::dvec3 from, glm::dvec3 to)
{
	glm::dvec3 from_nrm = glm::normalize(from);
	glm::dvec3 to_nrm = glm::normalize(to);

	if(glm::length(from - to) <= 0.00001)
	{
		return glm::dmat4(1);
	}

	glm::dvec3 axis = glm::normalize(glm::cross(from_nrm, to_nrm));
	double dot = glm::dot(from_nrm, to_nrm);
	double angle = glm::acos(dot);

	return glm::rotate(angle, axis);
}

glm::dvec3 MathUtil::spherical_to_euclidean(glm::dvec3 spherical)
{
	return spherical_to_euclidean(spherical.x, spherical.y, spherical.z);
}

glm::dvec3 MathUtil::spherical_to_euclidean(double azimuth, double inclination, double radius)
{
	glm::dvec3 out;

	out.x = sin(inclination) * cos(azimuth);
	out.y = cos(inclination);
	out.z = sin(inclination) * sin(azimuth);

	return radius * out;
}

glm::dvec3 MathUtil::euclidean_to_spherical(glm::dvec3 eu)
{
	glm::dvec3 out;

	out.z = glm::sqrt(eu.x * eu.x + eu.y * eu.y + eu.z * eu.z);
	out.x = atan2(eu.z, eu.x);
	out.y = glm::acos(eu.y / out.z);

	return out;
}

glm::dvec3 MathUtil::spherical_to_euclidean_r1(glm::dvec2 spherical)
{
	return spherical_to_euclidean_r1(spherical.x, spherical.y);
}

glm::dvec3 MathUtil::spherical_to_euclidean_r1(double azimuth, double inclination)
{
	glm::dvec3 out;

	out.x = sin(inclination) * cos(azimuth);
	out.y = cos(inclination);
	out.z = sin(inclination) * sin(azimuth);

	return out;
}

glm::dvec2 MathUtil::euclidean_to_spherical_r1(glm::dvec3 eu)
{
	glm::dvec2 out;

	out.x = atan2(eu.z, eu.x);
	out.y = glm::acos(eu.y);

	return out;
}

static glm::dquat quat_look_at_int(glm::dvec3 dir_nrm, glm::dvec3 up)
{
	glm::dmat3 res = glm::dmat3();

	// https://github.com/g-truc/glm/pull/659
	res[2] = -dir_nrm;
	res[0] = glm::normalize(glm::cross(up, res[2]));
	res[1] = glm::cross(res[2], res[0]);

	return glm::quat_cast(res);
}

glm::dquat MathUtil::quat_look_at(glm::dvec3 from, glm::dvec3 to, glm::dvec3 up, glm::dvec3 alt_up)
{
	glm::dvec3 dir = to - from;
	double dir_length = glm::length(dir);

	if(!(dir_length >= 0.000001))
	{
		logger->error("Given direction was not correct (too short or NaN)");
		return glm::dquat(1.0, 0.0, 0.0, 0.0);
	}

	dir /= dir_length;

	if (glm::abs(glm::dot(dir, up)) > 0.999)
	{
		return quat_look_at_int(dir, alt_up);
	}
	else
	{
		return quat_look_at_int(dir, up);
	}
}

double MathUtil::distance_to_line(glm::dvec3 a, glm::dvec3 b, glm::dvec3 p)
{
	glm::dvec3 ab = b - a;
	glm::dvec3 ap = p - a;

	/*if (glm::dot(ap, ab) <= 0.0)
	{
		return glm::length(ap);
	}

	glm::dvec3 bp = p - b;

	if (glm::dot(bp, ab) >= 0.0)
	{
		return glm::length(bp);
	}*/

	return glm::length(glm::cross(ap, ab)) / glm::length(ab);
}

int MathUtil::int_repeat(int v, int max)
{
	if (v < 0)
	{
		return (max + (-v % max)) % max;
	}
	else
	{
		return v % max;
	}
}

int MathUtil::int_clamp(int v, int max)
{
	if (v < 0)
	{
		return 0;
	}
	else
	{
		return glm::min(v, max);
	}
}

std::pair<glm::dvec3, glm::dvec3> MathUtil::screen_raycast(glm::dvec2 mouse_pos, 
		glm::dmat4 inv_tform, double distance)
{
	
	// This is in the near plane
	glm::dvec4 ray_start_ndc = glm::dvec4(mouse_pos, -1.0, 1.0);
	// This is in the far plane, potentially very very far away 
	glm::dvec4 ray_end_ndc = glm::dvec4(mouse_pos, 0.0, 1.0);

	glm::dvec4 ray_start4 = inv_tform * ray_start_ndc; ray_start4 /= ray_start4.w;
	glm::dvec4 ray_end4 = inv_tform * ray_end_ndc; ray_end4 /= ray_end4.w;
	glm::dvec3 ray_start = ray_start4, ray_rend = ray_end4;
	glm::dvec3 direction = glm::normalize(ray_rend - ray_start);
	glm::dvec3 ray_end = ray_start + direction * distance;

	return std::make_pair(ray_start, ray_end);
}

