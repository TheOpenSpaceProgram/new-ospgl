#include "SystemElement.h"


float SystemElement::get_dot_factor(float distance, float fov)
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

glm::dmat4 SystemElement::build_rotation_matrix(double t0, double t, bool include_at_epoch) const
{
	glm::dmat4 rot_matrix = glm::dmat4(1.0);

	double rot_angle = get_rotation_angle(t0, t, include_at_epoch);
	rot_matrix = glm::rotate(rot_matrix, rot_angle, rotation_axis);
	// Align pole to rotation axis
	rot_matrix = rot_matrix * MathUtil::rotate_from_to(glm::dvec3(0.0, 1.0, 0.0), rotation_axis);


	return rot_matrix;
}

double SystemElement::get_rotation_angle(double t0, double t, bool include_rot_at_epoch) const
{
	double offset = include_rot_at_epoch ? rotation_at_epoch : 0.0;
	double rot_angle = glm::mod(glm::radians(offset + t * rotation_speed), glm::two_pi<double>());
	double big_rot_angle = glm::mod(glm::radians(offset + t0 * rotation_speed), glm::two_pi<double>());
	return rot_angle + big_rot_angle;
}

double SystemElement::get_small_rotation_angle(double t0, double t, bool include_rot_at_epoch) const
{
	double offset = include_rot_at_epoch ? rotation_at_epoch : 0.0;
	double rot_angle = glm::mod(glm::radians(offset + t * rotation_speed), glm::two_pi<double>());
	return rot_angle;
}

glm::dvec3 SystemElement::get_tangential_speed(glm::dvec3 relative) const
{
	double dist_to_axis = MathUtil::distance_to_line(-rotation_axis, rotation_axis, relative);

	double speed_mod = dist_to_axis * glm::radians(rotation_speed);

	glm::dvec3 tg = glm::normalize(glm::cross(rotation_axis, relative));

	return tg * speed_mod;
}


SystemElement::SystemElement()
{
	dot_factor = 1.0f;
}


SystemElement::~SystemElement()
{
}

