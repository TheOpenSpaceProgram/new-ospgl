// Implements common atmosphere functionality for stuff inside atmospheres,

uniform bool do_atmo;

#include <core:shaders/atmosphere/atmo_util.fsi>

vec3 atmo(vec3 light_dir, vec3 camera_pos)
{
	vec3 start = camera_pos;

	if(length(vPosNrm) > atmo_radius * 1.5)
	{
		return vec3(0.0, 0.0, 0.0);
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
	float r_odepth = 0.0;
	vec3 total_r = vec3(0.0);
	float dr = length(ray);
	ray = normalize(ray);

	// Similar to atmo but the ray always ends at the surface
	for(int i = 0; i < GSTEPS; i++)
	{
		float cur_step = float(i) * GSTEPS_INVERSE;
		float next_step = float(i + 1) * GSTEPS_INVERSE;
		float step = min((cur_step + next_step) * 0.5f, 1.0f);

		ipos = start + ray * dr * step;

		float h = height(ipos);

		float odstep = density(h) * dr * GSTEPS_INVERSE;
		r_odepth += odstep;

		vec3 sray = normalize(-light_dir);
		vec2 sintersect = raySphereIntersect(ipos, sray, atmo_radius);

		float sdr = sintersect.y; // intersect.x doesn't make sense here as we start inside the atmo
		float sr_odepth = 0.0;

		float shadow = 0.0;
		for(int j = 0; j < SUB_GSTEPS; j++)
		{
			float sstep = float(j) * SUB_GSTEPS_INVERSE;
			vec3 spos = ipos + sray * sdr * sstep;
			float sh = height(spos);
			sr_odepth += density(sh) * sdr * SUB_GSTEPS_INVERSE;

			// Sum and average avoids some artifacts, we could
			// also do multiplication or even just sample shadow once!
			// The last parameter controls the "sharpness" of the shadow,
			// the greater the sharpest
			shadow += softSphereShadow(spos, sray, planet_radius * 0.8, 1.0);
		}
		shadow *= SUB_GSTEPS_INVERSE;
		// Tweak the iORlh parameters
		vec3 attn = exp(-kRlh * (r_odepth * 2e4 + sr_odepth * 3e6));
		total_r += attn * odstep * shadow;

	}

	vec3 color = total_r * kRlh * 1e6 * 2.0;
	return color;
}

