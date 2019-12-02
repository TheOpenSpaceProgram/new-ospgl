#version 330
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 vTex;

uniform mat4 tform;

// XY -> Texture offset (0->1)
// ZW -> Texture size (0->1)
uniform vec4 texoff;

void main()
{
	vTex = vec2(aTex.x * texoff.z + texoff.x, aTex.y * texoff.w + texoff.y);
	gl_Position = tform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}