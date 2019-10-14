#version 330 core

out vec4 FragColor;
  
in vec3 vColor;
in vec3 vNormal;
in vec3 vPos;

in float flogz;

uniform float f_coef;
uniform vec3 camera_pos;


void main()
{

    vec3 lightDir = normalize(vec3(-0.4, -1.0, -0.4));
    float diff = max(dot(-lightDir, vNormal), 0.1);

    vec3 viewDir = normalize(camera_pos - vPos);
    vec3 reflectDir = reflect(lightDir, vNormal); 

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float specular = 1.0 * spec; 

    vec3 col = vec3(0.5, 0.5, 0.9);

    FragColor = vec4(col * (diff + specular), 1.0);

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}