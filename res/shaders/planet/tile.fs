#version 330 core

out vec4 FragColor;
  
in vec3 vColor;
in vec3 vNormal;
in vec3 vPos;

in float flogz;

uniform float f_coef;


void main()
{

    vec3 lightDir = normalize(vec3(-0.4, -1.0, -0.4));
    float diff = max(dot(-lightDir, vNormal), 0.1);

    vec3 col = vec3(0.9, 0.9, 0.8);

    FragColor = vec4(diff * col, 1.0);

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}