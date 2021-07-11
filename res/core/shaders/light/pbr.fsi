const float PI = 3.14159265359;
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}
float GeometryGGXSchlick(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float nom    = NdotV;
	float denom = NdotV * (1.0 - k) + k ;

	return nom / denom;
}
float GeometryGGXSchlickIBL(float NdotV, float roughness)
{
	float a = roughness*roughness;
	float k = a / 2.0;

	float nom    = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
float GeometryGGX(float NdotV, float NdotL, float roughness)
{
	float ggx2 = GeometryGGXSchlick(NdotV, roughness);
	float ggx1 = GeometryGGXSchlick(NdotL, roughness);

	return ggx1 * ggx2;
}
float GeometryGGXIBL(float NdotV, float NdotL, float roughness)
{
	float ggx1 = GeometryGGXSchlickIBL(NdotV, roughness);
	float ggx2 = GeometryGGXSchlickIBL(NdotL, roughness);

	return ggx1 * ggx2;
}
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
vec3 FresnelSphericalGaussian(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * cosTheta - 6.98316 * cosTheta));
}


vec3 get_ambient(vec3 FragPos, vec3 Normal, vec3 Albedo, float Roughness, float Metallic,
    samplerCube irradiance_map, samplerCube specular_map, sampler2D brdf_map, out vec3 spec)
{

    vec3 cam_dir = normalize(-FragPos);
    Roughness = clamp(Roughness, 0.0, 1.0);
    Metallic = clamp(Metallic, 0.0, 1.0);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, Albedo, Metallic);
    vec3 F = FresnelSchlickRoughness(max(dot(Normal, cam_dir), 0.0), F0, Roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - Metallic;

    vec3 ambientkS = FresnelSchlick(max(dot(Normal, cam_dir), 0.0), F0);
    vec3 ambientkD = 1.0 - ambientkS;
    ambientkD *= 1.0 - Metallic;
    ambientkD = max(ambientkD, 0.05);

    vec3 ambient = texture(irradiance_map, Normal).rgb * Albedo * ambientkD;

    vec2 brdf_uv = vec2(max(dot(Normal, cam_dir), 0.0));
    brdf_uv = brdf_uv * 0.98;

    vec2 envBRDF = texture(brdf_map, brdf_uv, Roughness).rg;
    vec3 R = -reflect(-FragPos, Normal);
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(specular_map, R, Roughness * MAX_REFLECTION_LOD).rgb;
    spec = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    return ambient;
}

vec3 get_pbr(vec3 sun_dir, vec3 FragPos, vec3 Normal, vec3 Albedo, float Roughness, float Metallic)
{
    vec3 cam_dir = normalize(-FragPos);
    Roughness = clamp(Roughness, 0.0, 1.0);
    Metallic = clamp(Metallic, 0.0, 1.0);

    vec3 H = normalize(cam_dir + sun_dir);
    float NDF = DistributionGGX(Normal, H, Roughness);
    float G = max(GeometryGGX(max(dot(Normal, cam_dir), 0.0), max(dot(Normal, sun_dir), 0.0), Roughness), 0.001);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, Albedo, Metallic);
    vec3 F = FresnelSchlickRoughness(max(dot(Normal, cam_dir), 0.0), F0, Roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - Metallic;

    vec3 nominator = NDF * G * F;
    float denominator = 4 * max(dot(Normal, cam_dir), 0.0) * max(dot(Normal, sun_dir), 0.0);
    vec3 specular = nominator / max(denominator, 0.001);

    float NdotL = max(dot(Normal, sun_dir), 0.0);

    vec3 l0 = (kD * Albedo / PI + specular) * NdotL;

    return l0;
}