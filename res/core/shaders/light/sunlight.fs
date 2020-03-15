#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gEmissive;

uniform vec3 sun_pos;

void main()
{             
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    float Emissive = texture(gEmissive, TexCoords).r;

    vec3 sun_dir = normalize(sun_pos - FragPos);
    float diff = max(dot(Normal, sun_dir), 0.0);

    // Specular is super simple as the camera is always on (0, 0, 0)
    vec3 view_dir = normalize(-FragPos);
    vec3 reflect_dir = reflect(-sun_dir, Normal);

    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 128) * Specular;

    float total = max(diff, Emissive);

    FragColor = vec4(total * Albedo, 1.0);
}  