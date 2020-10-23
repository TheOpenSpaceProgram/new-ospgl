#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube tex;
uniform float tex_size;
uniform float roughness;

uniform sampler1D sample_positions;

const float PI = 3.14159265359;

float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001);
}

void main()
{
    vec3 N = normalize(localPos);
    vec3 R = N;
    vec3 V = R;

    if(roughness <= 0.01)
    {
        FragColor = vec4(texture(tex, N).rgb, 1.0);
    }
    else
    {
        // from tangent-space vector to world-space sample vector
        vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
        vec3 tangent   = normalize(cross(up, N));
        vec3 bitangent = cross(N, tangent);

        mat3 tangentToWorld = mat3(tangent, bitangent, N);

        const uint SAMPLE_COUNT = 32u;
        float totalWeight = 0.0;
        vec3 prefilteredColor = vec3(0.0);
        for(uint i = 0u; i < SAMPLE_COUNT; ++i)
        {
            // We must decode the data as textures cannot store negatives
            vec3 Hi  = (texelFetch(sample_positions, int(i), 0).rgb - 0.5) * 2.0;
            vec3 H = normalize(tangentToWorld * Hi);
            float VdotH = clamp(dot(V, H), 0.0, 1.0);
            vec3 L  = normalize(2.0 * VdotH * H - V);

            float NdotL = max(dot(N, L), 0.0);
            if(NdotL > 0.0)
            {
                float NdotH = clamp(dot(N, H), 0.0, 1.0);

                float D = DistributionGGX(NdotH, roughness);
                float Pdf = (D * NdotH / (4.0 * VdotH)) + 0.0001;
                float OmegaS = 1.0 / (float(SAMPLE_COUNT) * Pdf + 0.0001);
                float OmegaP = 4.0 * PI / (6.0 * tex_size * tex_size);
                float MipLevel = max(0.5 * log2(OmegaS / OmegaP) - 1.0, 0.0);

                prefilteredColor += textureLod(tex, L, MipLevel).rgb * NdotL;
                totalWeight      += NdotL;
            }
        }
        prefilteredColor = prefilteredColor / totalWeight;

        FragColor = vec4(prefilteredColor, 1.0);
    }
}