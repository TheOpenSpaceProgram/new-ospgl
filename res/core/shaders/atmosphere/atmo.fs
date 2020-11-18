#version 430 core

out vec4 FragColor;

in vec3 vPos;

in float flogz;
uniform float f_coef;

uniform vec3 camera_pos;

const int STEPS = 3;
const float STEP_INVERSE = 1.0 / float(STEPS);

uniform vec3 light_dir;

#include <core:shaders/atmosphere/atmo_util.fsi>

void main()
{

	vec3 vPosNrm = normalize(vPos);

	vec3 ray = -normalize(camera_pos - vPosNrm);

	float off = 0.0;

	if(length(camera_pos) <= 1.0)
	{
		off = -planet_radius;
	}

	vec2 planetIntersect = raySphereIntersect(camera_pos, ray, planet_radius + off);
	vec2 atmoIntersect = raySphereIntersect(camera_pos, ray, 1.0);

	// If we intersect the planet then atmo end-point is planet start point
	// intersect.x is distance to first intersection
	// intersect.y is distance to second intersection
	vec2 intersect;


	float planet = 0.0;

	if(planetIntersect != vec2(-1.0, -1.0))
	{
		intersect = vec2(atmoIntersect.x, planetIntersect.x);
		discard;
	}
	else
	{
		planet = 1.0;
		intersect = vec2(atmoIntersect.x, atmoIntersect.y);
	}


	if(length(camera_pos) <= 1.0)
	{
		// We start at camera_pos
		intersect.x = 0.0;
	}

    float r_odepth = 0.0;
    vec3 total_r = vec3(0.0);

	vec3 start = camera_pos + intersect.x * ray;
	vec3 ipos = start;

    vec3 kRlh = vec3(5.5e-6, 13.0e-6, 22.4e-6);

    // This is a cast from the camera to the points of the atmosphere,
    // we absorb the reflected sun-light
	for(int i = 0; i < STEPS; i++)
	{
        float step = float(i) / float(STEPS);
        float dr = intersect.y - intersect.x;

        ipos = start + ray * dr * step;

		float h = height(ipos);

        float odstep = density(h) * dr * STEP_INVERSE;
		r_odepth += odstep;

        vec3 sray = normalize(-light_dir);
        vec2 sintersect = raySphereIntersect(ipos, sray, 1.0);

        float sdr = sintersect.y; // intersect.x doesn't make sense here as we start inside the atmo
        float sr_odepth = 0.0;

        float shadow = 0.0;
		for(int j = 0; j < STEPS; j++)
		{
		    float sstep = float(j) / float(STEPS);
		    vec3 spos = ipos + sray * sdr * sstep;
		    float sh = height(spos);
		    sr_odepth += density(sh) * sdr * STEP_INVERSE;

            // Sum and average avoids some artifacts, we could
            // also do multiplication or even just sample shadow once!
            // The last parameter controls the "sharpness" of the shadow,
            // the greater the sharpest
            shadow += softSphereShadow(spos, sray, planet_radius, 1.0);
		}
		shadow /= STEPS;

        // Tweak the iORlh parameters
		vec3 attn = exp(-kRlh * (r_odepth * 2e4 + sr_odepth * 3e6));
        total_r += attn * odstep * shadow;

	}

    vec3 color = total_r * kRlh * 1e6;
	FragColor = vec4(color, 1.0);

	//gl_FragDepth = log2(flogz) * f_coef * 0.5;
}
































































































































































