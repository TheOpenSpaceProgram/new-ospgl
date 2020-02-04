#pragma once

struct LightingUniforms
{
	glm::dvec3 sun_pos;

	LightingUniforms()
	{
		// TODO: Remove this, create a Lighting system
		sun_pos = glm::dvec3(0.0, 0.0, 0.0);
	}
};