#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexture;

uniform mat4 tform;

uniform float f_coef;

out vec2 vTexture;

void main()
{
    gl_Position = vec4(aPos, 1.0f);
	vTexture = aTexture;
} 