// Implements common atmosphere functionality for stuff inside atmospheres,

uniform bool do_atmo;

#include <core:shaders/atmosphere/atmo_util.fsi>

vec3 atmo(vec3 light_dir, vec3 camera_pos)
{
	vec3 color = kRlh * 2e4;
	color *= max(0.15 - dot(vPosNrm, light_dir), 0.0);

	return color;
}

