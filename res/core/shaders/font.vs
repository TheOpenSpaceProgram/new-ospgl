#version 330
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 vTex;

uniform mat4 tform;

uniform vec2 scale;

// XY -> Texture offset (0->1)
// ZW -> Texture size (0->1)
uniform vec4 texoff;

void main()
{
	vec2 uv;
	
	if(scale.x < 0)
	{
		uv.y = (1.0 - aTex.x) * texoff.z + texoff.x;
	}
	else
	{
		uv.x = aTex.x * texoff.z + texoff.x;
	}

	if(scale.y < 0)
	{
		uv.y = (1.0 - aTex.y) * texoff.w + texoff.y;
	}
	else
	{
		uv.y = aTex.y * texoff.w + texoff.y;
	}

	vTex = uv;
	gl_Position = tform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}