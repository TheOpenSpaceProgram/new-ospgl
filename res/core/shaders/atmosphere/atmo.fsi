// Implements common atmosphere functionality for stuff inside atmospheres,
// the atmosphere shader itself is cheap.fs

uniform bool do_atmo;

#include <core:shaders/atmosphere/atmo_util.fsi>

vec4 atmo(vec3 lightDir, vec3 camera_pos)
{
	vec3 start = camera_pos;

	if(length(vPosNrm) > atmo_radius * 1.5)
	{
		return vec4(0.0, 0.0, 0.0, 0.0);
	}

	vec3 ray = vPosNrm - start;

	if(length(start) > atmo_radius * 1.0)
	{
		// Adjust start to intersection with atmosphere
		vec2 intersect = raySphereIntersect(start, ray, atmo_radius * 1.0);
		start = start + intersect.x * ray;
		ray = vPosNrm - start;
	}

	vec3 ipos = start;
	float d = 0.0;

	float step_size = length(ray) / float(ATMO_STEPS - 1);
	float last_d = 0.0;


	float dotp = exp(20.0 * dot(ray, vPosNrm)) * 16.0;

	for(int i = 0; i < ATMO_STEPS; i++)
	{
		float step = float(i) / float(ATMO_STEPS - 1);
		ipos = start + ray * step;

		float h = height(ipos);

		d += density(h) * step_size * (0.25 + dotp);
	}


  	float fade_factor = 0.25;
	  float fade_factor_add = 0.0;

	  float fade = max( min( dot(normalize(vPosNrm), -lightDir) + 0.1, fade_factor), 0.0) * (1.0 / fade_factor) + fade_factor_add;
	  d = min(pow(d, 0.5) * min(fade, 0.5) * 2.0, 1.0);

	float r_color = exp(-sunset_exponent * fade);

	vec3 col = atmo_main_color * (1.0 - r_color) + atmo_sunset_color * r_color;

	return vec4(col, atmo_curve(d));
}

