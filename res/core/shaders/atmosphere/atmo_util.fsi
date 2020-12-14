uniform vec3 atmo_main_color;
uniform vec3 atmo_sunset_color;
uniform float atmo_exponent;
uniform float sunset_exponent;
uniform vec3 kRlh;
uniform float kMie;
const int STEPS = 2;
const int SUB_STEPS = 10;
const int GSTEPS = 4;
const int SUB_GSTEPS = 10;
const float STEPS_INVERSE = 1.0 / float(STEPS);
const float SUB_STEPS_INVERSE = 1.0 / float(SUB_STEPS);
const float GSTEPS_INVERSE = 1.0 / float(GSTEPS);
const float SUB_GSTEPS_INVERSE = 1.0 / float(SUB_GSTEPS);
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

// Returns a value from 0->1, 0 being fully occluded
float softSphereShadow(vec3 r0, vec3 rd, float sr, float lr)
{
	float b = dot( r0, rd );
	float c = dot( r0, r0 ) - sr*sr;
	float h = b*b - c;

    // A bit expensive to compute, there are optimizations possible
    float d = sqrt( max(0.0,sr*sr-h)) - sr;
    float t = -b - sqrt( max(h,0.0) );
    return (t<0.0) ? 1.0 : smoothstep(0.0, 1.0, 2.5*lr*d/t );
}
