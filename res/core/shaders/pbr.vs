#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNrm;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aTgt;
layout (location = 4) in vec3 aBtg;


uniform mat3 normal_model;
uniform mat4 final_tform;
uniform mat4 deferred_tform;
uniform vec3 camera_relative;

uniform float f_coef;

out vec3 vPos;
out vec3 vNrm;
out vec2 vTex;
out float flogz;
out mat3 TBN;
out vec3 vTgt;

void main()
{
    gl_Position = final_tform * vec4(aPos, 1.0f);
	gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * f_coef - 1.0;
	flogz = 1.0 + gl_Position.w;

	vPos = (deferred_tform * vec4(aPos, 1.0f)).xyz;
	vNrm = normal_model * aNrm;
	vTex = aTex;

	vec3 T = normalize(vec3(deferred_tform * vec4(aTgt, 0.0)));
	vec3 B = normalize(vec3(deferred_tform * vec4(aBtg, 0.0)));
	vec3 N = normalize(vec3(deferred_tform * vec4(aNrm, 0.0)));

	TBN = mat3(T, B, N);
	vTgt = aTgt;

}
