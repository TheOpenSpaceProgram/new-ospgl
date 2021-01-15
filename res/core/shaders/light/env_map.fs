#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPbr;

uniform samplerCube irradiance_map;
uniform samplerCube specular_map;
uniform sampler2D brdf_map;

#include <core:shaders/light/pbr.fsi>

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

    vec3 specular; //< set by get_ambient
    vec3 ambient = get_ambient(FragPos, Normal, Albedo, Roughness, Metallic, irradiance_map, specular_map, brdf_map, specular);

    FragColor = vec4((ambient + specular) * Occlussion, 1.0);
    //FragColor = vec4(kD, 1.0);
}
