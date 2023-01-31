#version 430 core

out vec4 FragColor;
in float flogz;

uniform float f_coef;
uniform vec4 color;

void main()
{
    FragColor = color;
    // Logarithmic depth buffer
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}
