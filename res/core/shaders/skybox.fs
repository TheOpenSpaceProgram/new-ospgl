#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform float intensity;

void main()
{
    FragColor = vec4(texture(skybox, TexCoords).rgb * intensity, 1.0);
}

