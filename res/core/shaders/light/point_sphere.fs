#version 330 core
out vec4 FragColor;
  
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gEmissive;

uniform vec3 light_pos;
uniform vec3 color;
uniform vec3 spec_color;

uniform float p_constant;
uniform float p_linear;
uniform float p_quadratic;

uniform vec2 screen_size;

// Emissive and others are done in sunlight always
void main()
{             
    vec2 TexCoords = gl_FragCoord.xy / screen_size;

    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    float distance = length(light_pos - FragPos);
    float attenuation = 1.0f / (p_constant + distance * p_linear + distance * distance * p_quadratic);

    vec3 light_dir = normalize(light_pos - FragPos);
    vec3 diff = max(dot(Normal, light_dir), 0.0) * color * Albedo * attenuation;

    vec3 view_dir = normalize(-FragPos);
    vec3 reflect_dir = reflect(-light_dir, Normal);
    vec3 spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32) * Specular * spec_color * attenuation;

    FragColor = vec4(diff + spec, 1.0);
}  