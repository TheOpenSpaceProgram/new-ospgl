#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPbr;

uniform int mode;

void main()
{
    // retrieve data from G-buffer
    vec4 FragPosEmit = texture(gPosition, TexCoords).rgba;
	vec3 FragPos = FragPosEmit.rgb;
    float Emissive = FragPosEmit.a;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
    vec3 Pbr = texture(gPbr, TexCoords).rgb;
    float Occlussion = Pbr.r;
    float Roughness = Pbr.g;
    float Metallic = Pbr.b;

    // Position
    if(mode == 1)
    {
        FragColor = vec4(FragPos, 1.0);
    }
    // Emission intensity
    else if(mode == 2)
    {
        FragColor = vec4(Emissive, abs(Emissive), abs(Emissive), 1.0);
    }
    // Emission with albedo
    else if(mode == 3)
    {
        FragColor = vec4(abs(Emissive) * Albedo, 1.0);
    }
    // Normals
    else if(mode == 4)
    {
        FragColor = vec4(Normal, 1.0);
    }
    // Albedo
    else if(mode == 5)
    {
        FragColor = vec4(Albedo, 1.0);
    }
    // Occlussion
    else if(mode == 6)
    {
        FragColor = vec4(Occlussion, Occlussion, Occlussion, 1.0);
    }
    // Metallic
    else if(mode == 7)
    {
        FragColor = vec4(Metallic, Metallic, Metallic, 1.0);
    }
    // Roughness
    else if(mode == 8)
    {
        FragColor = vec4(Roughness, Roughness, Roughness, 1.0);
    }
    else
    {
        FragColor = vec4(1, 0, 1, 1.0);
    }


}
