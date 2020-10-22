#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube tex;
uniform float tex_size;
uniform float roughness;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;


    return H;
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
            vec2 Xi = Hammersley(i, SAMPLE_COUNT);
            vec3 Hi  = ImportanceSampleGGX(Xi, N, roughness);
            vec3 H = normalize(tangentToWorld * Hi);
            vec3 L  = normalize(2.0 * dot(V, H) * H - V);

            float NdotL = max(dot(N, L), 0.0);
            if(NdotL > 0.0)
            {
                float NdotH = clamp(dot(N, H), 0.0, 1.0);
                float VdotH = clamp(dot(V, H), 0.0, 1.0);

                // TODO: Calculate ggx properly, this looks CLOSE
                // to the 1024 samples, but it's certainly different
                float alpha = 0.4;
                float cos2 = NdotH * NdotH;
                float tan2 = (1-cos2) / cos2;
                float term = alpha / (cos2 * (alpha * alpha + tan2));
                float ggx = (1.0 / PI) * term * term;
                float Pdf = ggx * NdotH / (4.0 * VdotH);
                float OmegaS = 1.0 / (SAMPLE_COUNT * Pdf);
                float OmegaP = 4.0 * PI / (6.0 * tex_size * tex_size);
                float MipBias = 1.0;
                float MipLevel = max(0.5 * log2(OmegaS / OmegaP) + MipBias, 0.0);

                prefilteredColor += textureLod(tex, L, MipLevel).rgb * NdotL;
                totalWeight      += NdotL;
            }
        }
        prefilteredColor = prefilteredColor / totalWeight;

        FragColor = vec4(prefilteredColor, 1.0);
    }
}