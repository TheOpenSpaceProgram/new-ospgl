#version 330
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 vTex;

uniform mat4 tform;

void main()
{
	vTex = aTex;
	gl_Position = tform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
