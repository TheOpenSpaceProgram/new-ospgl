#version 330 core

out vec4 FragColor;

in float flogz;
uniform float f_coef;

uniform sampler2D diffuse;

in vec3 vPos;
in vec3 vNrm;
in vec2 vTex;


uniform vec3 sunlight_dir;


void main()
{
    float diffusel = max(dot(vNrm, sunlight_dir), 0.01);

    FragColor = vec4(texture(diffuse, vTex).rgb * diffusel, 1.0);
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}