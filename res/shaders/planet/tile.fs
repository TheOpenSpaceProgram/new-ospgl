#version 330 core

out vec4 FragColor;

in vec3 vColor;
in vec2 vTexture;
in vec3 vNormal;
in vec3 vPos;

in float flogz;

uniform float f_coef;


uniform vec3 camera_pos;

const int ATMO_STEPS = 4;
const float ATMO_STEPS_INVERSE = 1.0 / float(ATMO_STEPS);

uniform bool do_atmo;
uniform float atmo_radius;

uniform vec3 atmo_main_color;
uniform vec3 atmo_sunset_color;

float height(vec3 p)
{
    return max((length(p) - 1.0) / (atmo_radius - 1.0), 0.0);
}

float density(float h)
{
   float SCALE_H = 16.0;
   return min(exp2(-h * SCALE_H), 1.0);
}


vec4 atmo(vec3 lightDir)
{
    vec3 start = camera_pos;

    if(length(camera_pos) > atmo_radius * 10.0)
    {
        start = normalize(camera_pos) * atmo_radius * 10.0;
    }

    vec3 ray = vPos - start;

    vec3 ipos = start;
    float d = 0.0;

    float step_size = length(ray) / float(ATMO_STEPS);
    float last_d = 0.0;
    for(int i = 0; i < ATMO_STEPS; i++)
    {
        float step = float(i) / float(ATMO_STEPS - 1);
        ipos = start + ray * step;

		float h = height(ipos);

		d += density(h) * ATMO_STEPS_INVERSE * step_size;
    }


	float fade_factor = 0.25;
	float fade_factor_add = 0.0;

	float fade = max( min( dot(vPos, -lightDir) + 0.1, fade_factor), 0.0) * (1.0 / fade_factor) + fade_factor_add;
	d = min(pow(d, 0.23) * min(fade, 1.0) * 1.6, 0.6);
    vec3 col = atmo_main_color;

    return vec4(col, d);
}

void main()
{

    vec3 lightDir = normalize(vec3(-0.4, -1.0, -0.4));
    float diff = max(dot(-lightDir, vNormal), 0.0);

    vec3 col = vec3(0.8, 0.6, 0.6);

    vec4 atmoc = atmo(lightDir);

    FragColor = vec4(diff * col + atmoc.xyz * atmoc.w, 1.0);

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}