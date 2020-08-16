#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 vPos;
in vec3 vNrm;
in vec2 vTex;

in float flogz;
uniform float f_coef;

uniform sampler2D diffuse;
uniform int drawable_id;

uniform float transparency;

#include <core:/shaders/screen_door.fsi>

void main()
{    
	screen_door_transparency(transparency);

    gPosition = vec4(vPos, 0.0f);
    gNormal = normalize(vNrm);
    gAlbedoSpec.rgb = texture(diffuse, vTex).rgb;
    gAlbedoSpec.a = 0.2;

   	gl_FragDepth = log2(flogz) * f_coef * 0.5;
}

