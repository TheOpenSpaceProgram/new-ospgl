#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexture;

uniform mat4 tform;
uniform mat4 m_tform;

uniform float f_coef;

out vec3 vColor;
out vec3 vNormal;
out vec2 vTexture;

// In planet coordinates, 0->1 origin centered
out vec3 vPos;


out float flogz;
void main()
{
    gl_Position = tform * vec4(aPos, 1.0);
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * f_coef - 1.0;
	flogz = 1.0 + gl_Position.w;

	vColor = aColor;
	vNormal = aNormal;
	vTexture = aTexture;
	vPos = vec3(m_tform * vec4(aPos, 1.0));
}