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

    float atm = min(pow(flogz, 0.88) / 50000.0, 0.95);
    vec3 col = vec3(0.8, 0.3, 0.3);
    vec3 atmo = vec3(0.9, 0.6, 0.45);

    FragColor = vec4(diff * col * (1.0 - atm) + atmo * atm, 1.0);

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}