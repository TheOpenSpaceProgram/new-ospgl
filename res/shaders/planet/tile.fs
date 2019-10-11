#version 330 core

out vec4 FragColor;
  
in vec3 vColor;
in vec3 vNormal;
in vec3 vPos;

void main()
{

    vec3 lightDir = normalize(vec3(-0.4, -1.0, -0.4));
    float diff = max(dot(-lightDir, vNormal), 0.1);

    FragColor = vec4(diff * vec3(0.8, 0.8, 0.8), 1.0);
}