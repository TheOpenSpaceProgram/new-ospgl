#include "PlanetaryBody.h"

#include <imgui/imgui.h>

float PlanetaryBody::get_dot_factor(float distance, float fov)
{
	// In factor of the screen
	constexpr float MIN_APERTURE = 0.0030f;
	constexpr float START_FADE_APERTURE = 0.010f;

	// Simple trigonometry problem, get the angle 'alpha' of a 
	// rectangle triangle whose opposite cathetus is the planet's radius,
	// contiguous cathetus is the distance and hypotenuse is unknown
	// (We use atan)

	if (distance <= config.radius * 1.5f)
	{
		return 0.0f;
	}

	float aperture = 2.0f * atanf((float)config.radius / distance);

	float takes = aperture / fov;

	return 1.0f - glm::smoothstep(MIN_APERTURE, START_FADE_APERTURE, takes);
}

glm::dmat4 PlanetaryBody::build_rotation_matrix(double t) const
{
	glm::dmat4 rot_matrix = glm::mat4(1.0);

	double rot_angle = glm::radians(rotation_at_epoch + t * rotation_speed);
	rot_matrix = glm::rotate(rot_matrix, rot_angle, rotation_axis);
	// Align pole to rotation axis
	rot_matrix = rot_matrix * MathUtil::rotate_from_to(glm::dvec3(0.0, 1.0, 0.0), rotation_axis);


	return rot_matrix;
}

PlanetaryBody::PlanetaryBody()
{
	dot_factor = 1.0f;
}


PlanetaryBody::~PlanetaryBody()
{
}
