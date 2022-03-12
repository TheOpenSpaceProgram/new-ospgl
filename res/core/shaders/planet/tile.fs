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

uniform vec3 light_dir;
uniform vec3 camera_pos;

uniform sampler2D cliff_tex;
uniform sampler2D top_tex;

// Matrix for triplanar mapping that adjusts world to planet space
uniform mat4 tri_matrix;
uniform mat4 tri_nrm_matrix;
// offset to avoid the detail texture moving with the camera.
uniform vec3 cam_offset;
uniform float detail_scale;
uniform float triplanar_power;
uniform float triplanar_y_mult;

void main()
{
    vec3 atmoc = atmo(light_dir, camera_pos);

    // Adjusted triplanar mapping
    vec3 adjusted_pos = (vec3(tri_matrix * vec4(vPosScaled, 1.0))) * detail_scale + cam_offset;
    vec3 adjusted_nrm = vec3(vec4(vNormalScaled, 1.0));

    // Generate UVs and take samples
    vec2 top_uv = vec2(adjusted_pos.x, adjusted_pos.z);
    vec2 right_uv = vec2(adjusted_pos.y, adjusted_pos.x);
    vec2 front_uv = vec2(adjusted_pos.y, adjusted_pos.z);
    vec3 top_sample = vec3(texture(top_tex, top_uv));
    vec3 right_sample = vec3(texture(cliff_tex, right_uv));
    vec3 front_sample = vec3(texture(cliff_tex, front_uv));

    // Now combine the samples with the normal to obtian the color
    // Reduce the y contrution to make cliffs more noticeable
   // adjusted_nrm.y *= triplanar_y_mult;
    vec3 blend_weights = normalize(pow(abs(adjusted_nrm), vec3(triplanar_power)));
    vec3 triplanar_color = blend_weights.x * right_sample + blend_weights.y * top_sample + blend_weights.z * front_sample;

    float detailed_weight = min(505000.0 * detail_scale / dot(vPos, vPos), 1.0);
    vec3 col = vColor * detailed_weight * triplanar_color + vColor * (1.0 - detailed_weight);
    col = vColor;

    gAlbedo = (col + atmoc) * 0.77;
    gNormal = vNormal;
    gPositionEmit = vec4(vPos, length(atmoc));
    gPbr = vec3(0.0, 0.8, 0.2); // Occlusion, rougness, metallic

    gPositionEmit.w = 1.0;
    gAlbedo = vPosScaled;
    gAlbedo = top_sample;
    //gAlbedo = adjusted_pos;
    //gAlbedo = vec3(adjusted_pos.z);
    //gAlbedo = normalize(adjusted_pos * adjusted_pos * adjusted_pos);
    // FragColor = vec4(diff * texture(tex, vTexture).xyz, 1.0);
    // FragColor = vec4(vTexture, 0.0, 1.0);

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}
