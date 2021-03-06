#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 tform;

void main()
{
    WorldPos = aPos;
    gl_Position = tform * vec4(aPos, 1.0);
}

