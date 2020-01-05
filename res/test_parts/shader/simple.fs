#version 330 core

out vec4 FragColor;

in float flogz;
uniform float f_coef;

uniform sampler2D diffuse;

in vec3 vPos;
in vec3 vNrm;
in vec2 vTex;





void main()
{
    FragColor = texture(diffuse, vTex);
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}