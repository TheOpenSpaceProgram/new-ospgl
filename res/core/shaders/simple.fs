#version 430 core

layout (location = 0) out vec4 gPositionEmit;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gPbr;

out vec4 FragColor;

in vec3 vPos;

in float flogz;
uniform float f_coef;


void main()
{
    gPositionEmit = vec4(vPos, 1.0);
    gNormal = vec3(0.0, 0.0, 0.0);
    gAlbedo = vec3(1.0, 1.0, 1.0);
    gPbr = vec3(0.0, 0.0, 0.0);

    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}


