#version 330 core

out vec4 FragColor;
  
in vec3 vColor;
in vec3 vNormal;
in vec3 vPos;

void main()
{

    vec3 lightDir = normalize(vec3(-0.4, -1.0, -0.4));
    float diff = max(dot(-lightDir, vNormal), 0.5);

    FragColor = vec4(diff * vNormal, 1.0);
}