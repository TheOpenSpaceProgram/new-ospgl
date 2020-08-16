#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 sun_dir;
uniform vec3 color;
uniform vec3 ambient_color;
uniform vec3 spec_color;


void main()
{             
    // retrieve data from G-buffer
    vec4 FragPosEmit = texture(gPosition, TexCoords).rgba;
	vec3 FragPos = FragPosEmit.rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    float Emissive = FragPosEmit.a;

	float diff_fac = dot(Normal, sun_dir);
    vec3 diff = max(diff_fac, 0.0f) * color * Albedo;

    // Specular is super simple as the camera is always on (0, 0, 0)
    vec3 view_dir = normalize(-FragPos);
    vec3 reflect_dir = reflect(-sun_dir, Normal);

    vec3 spec = pow(max(dot(view_dir, reflect_dir), 0.0), 64) * Specular * spec_color;
    vec3 ambient = ambient_color * Albedo;

    vec3 emit = Emissive * Albedo;

	vec3 final_color = diff + spec + ambient + emit;

	// To avoid having to read the depth buffer we can simply use normals,
	// these are always ATLEAST near the unit vector, so any value below a
	// sane maximum can be assumed to be empty space
	if(Normal.x <= 0.1)
	{
		FragColor = vec4(1.0, 0.0, 1.0, 0.0);	
	}
	else
	{
    	FragColor = vec4(final_color, 1.0);
	}
}  
