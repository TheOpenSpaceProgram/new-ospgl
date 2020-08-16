#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 tform;


void main()
{
    gl_Position = tform * vec4(aPos, 1.0f);
}

