#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec3 aGlobalTex;

uniform mat4 tform;
uniform mat4 m_tform;
uniform mat4 normal_tform;
uniform mat4 rotm_tform;

uniform float f_coef;

out vec3 vColor;
out vec3 vNormal;
out vec3 vNormalScaled;
out float vTexture;
out vec2 vGlobalUV;

// In planet coordinates, 0->1 origin centered
out vec3 vPos;
out vec3 vPosNrm;
out vec3 vPosScaled;

out float flogz;

uniform vec3 tile;

void main()
{
    gl_Position = tform * vec4(aPos, 1.0f);
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * f_coef - 1.0;
	flogz = 1.0 + gl_Position.w;

	vColor = aColor;
	vNormal = vec3(normal_tform * vec4(aNormal, 1.0));
	vPosNrm = vec3(rotm_tform * vec4(aPos, 1.0));

	vGlobalUV = aGlobalTex.xy;
    vTexture = aGlobalTex.z;

	vPos = (m_tform * vec4(aPos, 1.0)).xyz;
	vPosScaled = (rotm_tform * vec4(aPos, 1.0)).xyz;
	vNormalScaled = (vec4(aNormal, 1.0)).xyz;
}