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
    float diffusel = max(dot(vNrm, vec3(0.0, 1.0, 0.0)), 0.5);

    FragColor = vec4(texture(diffuse, vTex).rgb * diffusel, 1.0);
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}