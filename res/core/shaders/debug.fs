#version 330 core

out vec4 FragColor;

in vec4 vColor;
in vec3 vPos;

in float flogz;

uniform float f_coef;



void main()
{
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}