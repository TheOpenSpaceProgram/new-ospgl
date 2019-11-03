#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in float aDepth;
layout (location = 3) in vec2 aTexture;

uniform mat4 tform;
uniform mat4 tform_scaled;
uniform mat4 normal_tform;
uniform mat4 rotm_tform;

uniform int clockwise;
uniform float f_coef;

out vec3 vNormal;
out vec3 vPos;
out vec3 vPosNrm;
out vec2 vTexture;
out float flogz;
out float vDepth;

uniform vec3 tile;

uniform float time;

vec2 get_real_uv()
{
	return (aTexture / pow(2, tile.z) + tile.xy * 1000.0) * 0.001;
}

void main()
{
	vTexture = get_real_uv();

	vec4 wPos = tform_scaled * vec4(aPos, 1.0);

    gl_Position = tform * vec4(aPos, 1.0);
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * f_coef - 1.0;

	flogz = 1.0 + gl_Position.w;

	vPos = wPos.xyz;

	vNormal = vec3(normal_tform * vec4(aNormal, 1.0));
	vPosNrm = vec3(rotm_tform * vec4(aPos, 1.0));
	vDepth = aDepth;

}