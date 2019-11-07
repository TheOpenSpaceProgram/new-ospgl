#version 430 core

out vec4 FragColor;

in vec3 vColor;
in vec2 vTexture;
in vec3 vNormal;
in vec3 vPos;
in vec3 vPosNrm;
in vec3 vPosSphereNrm;

in float flogz;

uniform float f_coef;


uniform vec3 camera_pos;

const int ATMO_STEPS = 4;
const float ATMO_STEPS_INVERSE = 1.0 / float(ATMO_STEPS);

uniform bool do_atmo;
uniform float atmo_radius;

uniform vec3 atmo_main_color;
uniform vec3 atmo_sunset_color;
uniform float atmo_exponent;
uniform float sunset_exponent;

uniform vec3 light_dir;


float height(vec3 p)
{
    return max((length(p) - 1.0) / (atmo_radius - 1.0), 0.0);
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

vec4 atmo(vec3 lightDir)
{
    vec3 start = camera_pos;

    if(length(vPosNrm) > atmo_radius * 1.5)
    {
        return vec4(0.0, 0.0, 0.0, 0.0);
    }

    vec3 ray = vPosNrm - start;

    if(length(start) > atmo_radius * 1.5)
    {
        // Adjust start to intersection with atmosphere
        vec2 intersect = raySphereIntersect(start, ray, atmo_radius * 1.5);
        start = start + intersect.x * ray;
        ray = vPosNrm - start;
    }

    vec3 ipos = start;
    float d = 0.0;

    float step_size = length(ray) / float(ATMO_STEPS - 1);
    float last_d = 0.0;

    float dotp = exp(8.0 * dot(ray, vPosNrm)) * 4.0;

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
	  d = min(pow(d, 0.47) * min(fade, 0.5) * 2.0, 1.0);

    float r_color = exp(-sunset_exponent * fade);

    vec3 col = atmo_main_color * (1.0 - r_color) + atmo_sunset_color * r_color;

    return vec4(col, atmo_curve(d));
}

void main()
{
    vec4 atmoc = atmo(light_dir);

    float diff = max(dot(-light_dir, vNormal), atmoc.w);

    vec3 col = vColor;


    FragColor = vec4((diff * col + atmoc.xyz * atmoc.w) * 0.77, 1.0);

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
   gl_FragDepth = log2(flogz) * f_coef * 0.5;
}