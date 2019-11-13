#version 330
layout (location = 0) in vec3 aPos;


uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 vColor;
out vec2 vTex;
out vec3 vNormal;
out vec3 vPos;
out vec3 vPosOriginal;


vec2 get_projected_uv(vec3 p)
{
	const float pi = 3.14159265358979;
	return vec2((atan(p.z, p.x) + pi) / (2.0 * pi), acos(p.y) / pi);
}


void main()
{
	vTex = get_projected_uv(aPos);
	vNormal = mat3(transpose(inverse(model))) * aPos;
	vPos = vec3(model * vec4(aPos, 1.0));
	vPosOriginal = aPos;
	gl_Position = proj * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);

}