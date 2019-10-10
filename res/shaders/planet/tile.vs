#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexture;

uniform mat4 tform;

out vec3 vColor;
out vec3 vNormal;

void main()
{
    gl_Position = tform * vec4(aPos, 1.0f);
	vColor = aColor;
	vNormal = aNormal;
} 