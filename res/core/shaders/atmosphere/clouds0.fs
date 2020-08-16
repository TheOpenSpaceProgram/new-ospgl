#version 330 core

out vec4 FragColor;

in vec3 vPos;

in float flogz;
uniform float f_coef;

uniform vec3 camera_pos;
uniform float planet_radius;

const int STEPS = 2;
const float STEP_INVERSE = 1.0 / float(STEPS);

uniform vec3 atmo_main_color;
uniform vec3 atmo_sunset_color;
uniform float atmo_exponent;


void main()
{

	FragColor = vec4(vec3(1.0, 1.0, 1.0), 1.0);

	gl_FragDepth = log2(flogz) * f_coef * 0.5;
}