#version 430 core

layout (location = 0) in vec3 aPos;

uniform mat4 tform;

uniform float f_coef;

out vec3 vNormal;
out vec3 vPos;
out float flogz;

void main()
{
    gl_Position = tform * vec4(aPos, 1.0f);
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * f_coef - 1.0;
	flogz = 1.0 + gl_Position.w;

	vPos = aPos;

	vNormal = aPos;
}