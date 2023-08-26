#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gPbr;

uniform vec3 sun_dir;
uniform vec3 color;
uniform vec3 ambient_color;

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
	if(Normal.x <= 0.1)
	{
		FragColor = vec4(1.0, 0.0, 1.0, 0.0);
	}
	else
	{
        vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
        vec3 Pbr = texture(gPbr, TexCoords).rgb;
        float Occlussion = Pbr.r;
        float Roughness = Pbr.g;
        float Metallic = Pbr.b;

        vec3 emit = Emissive * Albedo;

        vec3 lo = get_pbr(sun_dir, FragPos, Normal, Albedo, Roughness, Metallic);

        // Env mapping
        vec3 specular; //< set by get_ambient
        vec3 ambient = get_ambient(FragPos, Normal, Albedo, Roughness, Metallic, Emissive, irradiance_map, specular_map, brdf_map, specular);

        vec3 fcolor = lo * color + emit + (specular + ambient) * Occlussion;

        // Apply fixed HDR
        const float gamma = 2.2;
        const float exposure = 3.0;

        vec3 mapped = vec3(1.0) - exp(-fcolor * exposure);
        mapped = pow(mapped, vec3(1.0 / gamma));

        FragColor = vec4(mapped, 1.0);
	}

}
