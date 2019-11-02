#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

uniform mat4 tform;

uniform float f_coef;

out vec4 vColor;
out vec3 vPos;
out float flogz;

void main()
{
    gl_Position = tform * vec4(aPos, 1.0f);
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * f_coef - 1.0;
	flogz = 1.0 + gl_Position.w;

	vPos = aPos;
	vColor = aColor;
}