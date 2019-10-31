#version 330 core

out vec4 FragColor;

in vec3 vPos;

in float flogz;
uniform float f_coef;

uniform vec3 camera_pos;
uniform float planet_radius;

const int STEPS = 2;
const float STEP_INVERSE = 1.0 / float(STEPS);

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

float rayPointDistance(vec3 r0, vec3 rd, vec3 p)
{
	vec3 a = r0;
	vec3 b = r0 + rd;

	return length(cross(p - a, p - b)) / length(b - a);
}


float height(vec3 p)
{
    return max((length(p) - planet_radius) / (planet_radius), 0.0);
}

float density(float h)
{
   float SCALE_H = 8.0 / (1.0 - planet_radius);
   return min(exp2(-h * SCALE_H), 1.0);
}


void main()
{
	vec3 lDir = normalize(vec3(-0.4, -1.0, -0.4));
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

	float d = 0.0;
	float l = 0.0;

	vec3 start = camera_pos + intersect.x * ray;
	vec3 ipos = start;

	for(int i = 0; i < STEPS; i++)
	{
        float step = float(i) / float(STEPS);
        float dr = intersect.y - intersect.x;

        ipos = start + ray * dr * step;

		float h = height(ipos);
		float shfac = 1.0;

		float dist = rayPointDistance(ipos, -lDir, vec3(0.0, 0.0, 0.0));

		float dotp = dot(ipos, -lDir);
		float dfac = exp(50.0 * dotp) / (exp(50.0 * dotp) + 1.0);

		dist = dfac + (1.0 - dfac) * dist;

		shfac = pow(dist, 80.0);



		d += density(h) * STEP_INVERSE * min(max(shfac, 0.0), 1.0);


	}

	float fade_factor = 0.05;
	float fade_factor_add = 0.0;

	float fade = max( min( dot(ipos, -lDir), fade_factor), 0.0) * (1.0 / fade_factor) + fade_factor_add;
	d = sqrt(d) * 1.5;

	vec3 col = d * vec3(0.6, 0.7, 1.5);

	float mie = max(dot(ray, -lDir), 0.0);
	mie = pow(mie, 128.0);

	float r_factor = dot(start, lDir);
	r_factor = 1.0 - (pow(r_factor, 64.0));

	vec3 mieColor = vec3(1.0, 1.0, 1.0) * (1.0 - r_factor) + vec3(1.0, 0.5, 0.3) * r_factor;

	FragColor = vec4(col + mie * mieColor * d, d);

	gl_FragDepth = log2(flogz) * f_coef * 0.5;
}