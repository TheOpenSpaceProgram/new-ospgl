#version 330 core

out vec4 FragColor;

in vec3 vPos;
in vec3 vNrm;

in float flogz;

uniform float f_coef;

uniform vec3 color;

uniform vec3 camera_relative;

const vec3 light_dir = vec3(1.0, 0.0, 0.0);

const float rim_start = 0.0;
const float rim_end = 1.0;


void main()
{
	vec3 eye = normalize(camera_relative - vPos);
	vec3 normal = normalize(vNrm);

	float d = dot(normal, eye);
	float diffuse = max(d, 0.0);
	
	float color_intensity = length(color);
	float gain = 1.73f;

	float rim = pow(smoothstep(0.0, 1.0, 1.0 - d), 1.0f / color_intensity);

	FragColor = vec4(normalize(color) * gain, max(rim, 0.2) * color_intensity);

    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}
