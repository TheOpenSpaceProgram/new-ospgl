#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gEmissive;

uniform vec3 sun_pos;
uniform vec3 color;
uniform vec3 ambient_color;
uniform vec3 spec_color;

void main()
{             
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    float Emissive = texture(gEmissive, TexCoords).r;

    vec3 sun_dir = normalize(sun_pos - FragPos);
    vec3 diff = max(dot(Normal, sun_dir), 0.0f) * color * Albedo;

    // Specular is super simple as the camera is always on (0, 0, 0)
    vec3 view_dir = normalize(-FragPos);
    vec3 reflect_dir = reflect(-sun_dir, Normal);

    vec3 spec = pow(max(dot(view_dir, reflect_dir), 0.0), 64) * Specular * spec_color;
    vec3 ambient = ambient_color * Albedo;

    vec3 emit = Emissive * Albedo;

    FragColor = vec4(diff + spec + ambient + emit, 1.0);
}  