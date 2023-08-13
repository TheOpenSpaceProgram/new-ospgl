#version 430 core
layout (location = 0) out vec4 gPositionEmit;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gPbr;

out vec4 FragColor;

in vec3 vColor;
in vec3 vNormal;
in vec3 vPos;
in vec3 vPosScaled;
in vec3 vNormalScaled;
in vec3 vPosNrm;
in vec3 vPosSphereNrm;
in vec2 vGlobalUV;
in float vTexture;

in float flogz;
uniform float f_coef;

#include <core:shaders/atmosphere/atmo.fsi>
#include <quality_defines>

uniform vec3 light_dir;
uniform vec3 camera_pos;

uniform sampler2D cliff_tex;
uniform sampler2D top_tex;
uniform sampler2D cliff_nrm;
uniform sampler2D top_nrm;

// Triplanar mapping stuff
#ifdef _USE_PLANET_DETAILS
uniform mat4 tri_matrix;
uniform mat4 tri_nrm_matrix;
uniform mat4 inverse_tri_nrm_matrix;
uniform float detail_scale;
uniform float detail_fade;
uniform float triplanar_power;
uniform float triplanar_y_mult;
#endif

uniform int do_detail;

void main()
{
    vec3 atmoc = atmo(light_dir, camera_pos);

    vec3 col = vColor;
    vec3 nrm = vNormal;

    #ifdef _USE_PLANET_DETAILS
    if(do_detail == 1)
    {
        // Adjusted triplanar mapping
        vec3 adjusted_pos = (vec3(tri_matrix * vec4(vPosScaled, 1.0))) * detail_scale;
        vec3 adjusted_nrm = vec3(tri_nrm_matrix * vec4(vNormal, 1.0));

        // Generate UVs and take samples
        vec2 xplane_uv = adjusted_pos.zy;
        vec2 yplane_uv = adjusted_pos.xz;
        vec2 zplane_uv = adjusted_pos.xy;

        #ifdef _USE_PLANET_DETAIL_MAP
        vec3 xplane_sample = texture(cliff_tex, xplane_uv).xyz;
        vec3 yplane_sample = texture(top_tex, yplane_uv).xyz;
        vec3 zplane_sample = texture(cliff_tex, zplane_uv).xyz;
        #endif

        #ifdef _USE_PLANET_DETAIL_NORMAL
        // Normals must go from -1 to 1!
        vec3 xplane_nrm = (texture(cliff_nrm, xplane_uv).xyz * 2.0 - vec3(1.0));
        vec3 yplane_nrm = (texture(top_nrm, yplane_uv).xyz * 2.0 - vec3(1.0));
        vec3 zplane_nrm = (texture(cliff_nrm, zplane_uv).xyz * 2.0 - vec3(1.0));

        vec3 nrm_x = vec3(0, xplane_nrm.yx);
        vec3 nrm_y = vec3(yplane_nrm.x, 0, yplane_nrm.y);
        vec3 nrm_z = vec3(zplane_nrm.xy, 0);

        #endif

        // Reduce the y contrution to make cliffs more noticeable (in blending only)
        vec3 orig_adjusted_nrm = adjusted_nrm;
        adjusted_nrm.y *= triplanar_y_mult;
        vec3 blend_weights = normalize(pow(abs(adjusted_nrm), vec3(triplanar_power)));
        float detailed_weight = min(detail_fade / dot(vPos, vPos), 1.0);

        #ifdef _USE_PLANET_DETAIL_MAP
        vec3 triplanar_color = blend_weights.x * xplane_sample +
            blend_weights.y * yplane_sample +
            blend_weights.z * zplane_sample;
        col = vColor + (detailed_weight * triplanar_color);
        #endif

        #ifdef _USE_PLANET_DETAIL_NORMAL
        vec3 tri_normal = normalize(nrm_x * blend_weights.x + nrm_y * blend_weights.y + nrm_z * blend_weights.z + orig_adjusted_nrm);
        nrm = tri_normal * detailed_weight + orig_adjusted_nrm * (1.0 - detailed_weight);
        // Transform the normal back into world space
        nrm = normalize((inverse_tri_nrm_matrix * vec4(nrm, 1.0)).xyz);
        #endif
    }
    #endif


    gAlbedo = (col + atmoc) * 0.77;
    gNormal = nrm;
    vec3 vPosFixed = vec3(-vPos.x, vPos.y, vPos.z);
    gPositionEmit = vec4(vPosFixed, -length(atmoc));
    // We use Occlussion to hide the environment map
    float dist = length(vPosNrm - camera_pos);
    // Note: Negative roughness value means disable fresnel!
    gPbr = vec3(max(1.0 - dist, 0.0), 0.85, 0.0); // Occlusion, rougness, metallic

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}
