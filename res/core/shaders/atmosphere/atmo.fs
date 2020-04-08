#version 330 core

out vec4 FragColor;

in vec3 vNormal;
// Relative to (0, 0). Scaled space (untransformed)
in vec3 vPos;

in float flogz;

const int OUT_STEPS = 3;
const int STEPS = 3;
const float STEP_CONTRIB = 1.0f / float(STEPS);

float K_R = 0.066;
const float K_M = 0.00055;
const vec3 C_R = vec3(0.3, 0.7, 1.0);
const float E = 14.3;
const float G_M = -0.85;

uniform float f_coef;
// Relative to (0, 0). Scaled space
uniform vec3 camera_pos;

uniform float planet_radius;
// Atmo radius is always 1.0

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

float density(float h)
{
   float SCALE_H = 8.0 / (1.0 - planet_radius);
   return exp(-h * SCALE_H);
}

float height(vec3 p)
{
    return (length(p) - planet_radius) / (planet_radius);
}

float optic(vec3 p, vec3 q)
{
    float SCALE_L = 1.0 / (1.0 - planet_radius);

	vec3 step = (q - p) / float(OUT_STEPS);
	vec3 v = p + step * 0.5;

	float sum = 0.0;
	for(int i = 0; i < OUT_STEPS; i++)
    {
		sum += density(height(v));
		v += step;
	}
	sum *= length(step) * SCALE_L;
	return sum;
}

// Reyleigh
// g : 0
// F = 3/4 * ( 1 + c^2 )
float rayleighPhase(float cc)
{
	return 0.75 * (1.0 + cc);
}


// Mie
// g : ( -0.75, -0.999 )
//      3 * ( 1 - g^2 )               1 + c^2
// F = ----------------- * -------------------------------
//      2 * ( 2 + g^2 )     ( 1 + g^2 - 2 * g * c )^(3/2)
float miePhase(float g, float c, float cc)
{
	float gg = g * g;

	float a = ( 1.0 - gg ) * ( 1.0 + cc );

	float b = 1.0 + gg - 2.0 * g * c;
	b *= sqrt( b );
	b *= 2.0 + gg;

	return 1.5 * a / b;
}


vec3 atmo(vec3 lightDir)
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



    if(planetIntersect != vec2(-1.0, -1.0))
    {
        intersect = vec2(atmoIntersect.x, planetIntersect.x);
    }
    else
    {
        intersect = vec2(atmoIntersect.x, atmoIntersect.y);
    }


    if(length(camera_pos) <= 1.0)
    {
        // We start at camera_pos
        intersect.x = 0.0;
    }

    vec3 acc;
    vec3 start = camera_pos + intersect.x * ray;

    float len = (intersect.y - intersect.x) / float(STEPS);

    vec3 v = start + ray * (len * 0.5);

    for(int i = 0; i < STEPS; i++)
    {
        vec2 f = raySphereIntersect(v, -lightDir, 1.0);

        float step = float(i) / float(STEPS);
        float dr = intersect.y - intersect.x;

        vec3 ipos = start + ray * dr * step;

        float h = height(ipos);

        vec3 u = v + lightDir * f.y;
        float n = (optic(start, v) + optic(v, u))*(3.14159 * 4.0);

       // acc += density(h) * vec3(1.0, 0.0, 1.0) * STEP_CONTRIB * 5.0;
        acc += density(h) * exp(-n * ( K_R * C_R + K_M ));
        v += ray * len;
    }

    float SCALE_L = 1.0 / (1.0 - planet_radius);


    acc *= len * SCALE_L;
	float c = dot(ray, -lightDir);
	float cc = c * c;
    vec3 col = acc * ( K_R * C_R * rayleighPhase(cc) + K_M * miePhase(G_M, c, cc)) * E;

    return col;
}

void main()
{

    vec3 atmoc = atmo(-normalize(vec3(-0.4, -1.0, -0.4)));
    FragColor = vec4(atmoc, 1.0);

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}