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

uniform vec3 sun_pos;
uniform vec3 color;
uniform vec3 ambient_color;

uniform sampler2D near_shadow_map;
uniform mat4 near_shadow_tform;

#include <core:shaders/light/pbr.fsi>

float calculate_shadow(vec4 FragPosLightSpace, float diff_fac)
{
	vec3 shadow_proj = FragPosLightSpace.xyz / FragPosLightSpace.w;
	shadow_proj = shadow_proj * 0.5 + 0.5;
	
	float bias = max(0.01 * (1.0 - diff_fac), 0.005);  

	if(shadow_proj.z > 1.0)
	{	
		return 1.0;
	}
	else
	{
		float closest_depth = texture(near_shadow_map, shadow_proj.xy).r;
		float current_depth = shadow_proj.z;
		//float shadow = current_depth - bias > closest_depth ? 0.0f : 1.0f;
		//return shadow;

		float shadow = 0.0;
		vec2 texel_size = 1.0 / textureSize(near_shadow_map, 0);
		for(int x = -1; x <= 1; ++x)
		{
    		for(int y = -1; y <= 1; ++y)
    		{
       			float pcf_depth = texture(near_shadow_map, shadow_proj.xy + vec2(x, y) * texel_size).r; 
        		shadow += current_depth - bias > pcf_depth ? 0.0 : 1.0;        
    		}    
		}
		shadow /= 9.0;
		return shadow;
	}	
}


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

    Roughness = 0.3;


    vec3 sun_dir = normalize(sun_pos - FragPos);
    // TODO: Environment map sampling
    vec3 emit = Emissive * Albedo;

    vec3 ambient, specular;
    vec3 lo = get_pbr(sun_dir, FragPos, Normal, Albedo, Roughness, Metallic, irradiance_map, specular_map, brdf_map,
        ambient, specular);

	vec4 FragPosLightSpace = near_shadow_tform * vec4(FragPos, 1.0f);
	float shadow = calculate_shadow(FragPosLightSpace, dot(Normal, sun_dir));

	vec3 fcolor = lo * shadow * color + (ambient + specular) * Occlussion + emit;

    FragColor = vec4(fcolor, 1.0);
    //FragColor = vec4(kD, 1.0);
}
