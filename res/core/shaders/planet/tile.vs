#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexture;

uniform mat4 tform;
uniform mat4 m_tform;
uniform mat4 normal_tform;
uniform mat4 rotm_tform;

uniform float f_coef;

out vec3 vColor;
out vec3 vNormal;
out vec2 vTexture;

// In planet coordinates, 0->1 origin centered
out vec3 vPos;
out vec3 vPosNrm;

out float flogz;

uniform vec3 tile;

vec2 get_real_uv()
{
	return (aTexture / pow(2, tile.z) + tile.xy * 1000.0) * 0.001;
}

vec2 get_projected_uv(vec3 p)
{
	const float pi = 3.14159265358979;
	return vec2((atan(p.z, p.x) + pi) / (2.0 * pi), acos(p.y) / pi);
}

void main()
{
    gl_Position = tform * vec4(aPos, 1.0f);
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * f_coef - 1.0;
	flogz = 1.0 + gl_Position.w;

	vColor = aColor;
	vNormal = vec3(normal_tform * vec4(aNormal, 1.0));
	vPosNrm = vec3(rotm_tform * vec4(aPos, 1.0));

	vPos = (m_tform * vec4(aPos, 1.0)).xyz;

	vTexture =  get_projected_uv(normalize(vPos));
}