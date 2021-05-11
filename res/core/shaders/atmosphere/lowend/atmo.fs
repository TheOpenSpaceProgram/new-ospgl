out vec4 FragColor;

in vec3 vPos;

in float flogz;
uniform float f_coef;

uniform vec3 camera_pos;
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
    float dr = intersect.y - intersect.x;


    // This is a cast from the camera to the points of the atmosphere,
    // we absorb the reflected sun-light
	for(int i = 0; i < STEPS; i++)
	{
        float step = float(i) * STEPS_INVERSE;

        ipos = start + ray * dr * step;

		float h = height(ipos);

        float r_odstep = density(h) * dr * STEPS_INVERSE;
		r_odepth += r_odstep;

		float shadow = softSphereShadow(ipos, -light_dir, planet_radius, 1.0);


		vec3 attn = exp(-(kRlh * r_odepth * 2e4));
        total_r += attn * r_odstep * shadow;


	}

    vec3 color = total_r * kRlh;
	FragColor = vec4(color * 2e6, 1.0);

	//gl_FragDepth = log2(flogz) * f_coef * 0.5;
}


