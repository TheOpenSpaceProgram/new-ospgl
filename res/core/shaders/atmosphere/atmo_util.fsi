uniform vec3 atmo_main_color;
uniform vec3 atmo_sunset_color;
uniform float atmo_exponent;
uniform float sunset_exponent;
const int ATMO_STEPS = 4;
const float ATMO_STEPS_INVERSE = 1.0 / float(ATMO_STEPS);
uniform float atmo_radius;
uniform float planet_radius;

float rayPointDistance(vec3 r0, vec3 rd, vec3 p)
{
	vec3 a = r0;
	vec3 b = r0 + rd;

	return length(cross(p - a, p - b)) / length(b - a);
}

float height(vec3 p)
{
	return max((length(p) - planet_radius) / (atmo_radius - planet_radius), 0.0);
}

float density(float h)
{
   float SCALE_H = 8.0;
   return min(exp(-pow(h, atmo_exponent) * SCALE_H), 1.0);
}

float atmo_curve(float d)
{
	return 1.1 * (pow(20, d) - 1) / (pow(20, d) + 1);
}


vec2 raySphereIntersect(vec3 r0, vec3 rd, float sr)
{
	float a = dot(rd, rd);
	vec3 s0_r0 = r0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sr * sr);
	float disc = b * b - 4.0 * a* c;

	if (disc < 0.0)
	{
		return vec2(-1.0, -1.0);
	}
	else
	{
		return vec2(-b - sqrt(disc), -b + sqrt(disc)) / (2.0 * a);
	}
}