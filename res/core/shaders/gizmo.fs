#version 330 core

out vec4 FragColor;

in vec3 vPos;
in vec3 vNrm;

in float flogz;

uniform float f_coef;

uniform vec3 color;
uniform float transparency;

uniform vec3 camera_relative;

const vec3 light_dir = vec3(1.0, 0.0, 0.0);

const float rim_start = 0.0;
const float rim_end = 1.0;


void main()
{
	vec3 eye = normalize(camera_relative - vPos);
	vec3 normal = normalize(vNrm);

	float diffuse = max(dot(normal, eye), 0.0);

	FragColor = vec4(color * diffuse, 1.0 - transparency);

    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}
