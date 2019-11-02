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

PlanetaryBody::PlanetaryBody()
{
	dot_factor = 1.0f;
}


PlanetaryBody::~PlanetaryBody()
{
}
