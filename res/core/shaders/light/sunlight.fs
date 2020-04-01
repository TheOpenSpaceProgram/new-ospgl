#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 sun_pos;
uniform vec3 color;
uniform vec3 ambient_color;
uniform vec3 spec_color;

uniform sampler2D near_shadow_map;
uniform mat4 near_shadow_tform;

float calculate_shadow(vec4 FragPosLightSpace, float diff_fac)
{
	vec3 shadow_proj = FragPosLightSpace.xyz / FragPosLightSpace.w;
	shadow_proj = shadow_proj * 0.5 + 0.5;
	
	float bias = max(0.0020 * (1.0 - diff_fac), 0.00005);  

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
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    float Emissive = FragPosEmit.a;

    vec3 sun_dir = normalize(sun_pos - FragPos);
	float diff_fac = dot(Normal, sun_dir);
    vec3 diff = max(diff_fac, 0.0f) * color * Albedo;

    // Specular is super simple as the camera is always on (0, 0, 0)
    vec3 view_dir = normalize(-FragPos);
    vec3 reflect_dir = reflect(-sun_dir, Normal);

    vec3 spec = pow(max(dot(view_dir, reflect_dir), 0.0), 64) * Specular * spec_color;
    vec3 ambient = ambient_color * Albedo;

    vec3 emit = Emissive * Albedo;

	vec4 FragPosLightSpace = near_shadow_tform * vec4(FragPos, 1.0f);
	
	float shadow = calculate_shadow(FragPosLightSpace, diff_fac);
	
	diff *= shadow;
	spec *= shadow;

    FragColor = vec4(diff + spec + ambient + emit, 1.0);

}  
