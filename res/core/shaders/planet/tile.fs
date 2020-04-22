#version 430 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

out vec4 FragColor;

in vec3 vColor;
in vec2 vTexture;
in vec3 vNormal;
in vec3 vPos;
in vec3 vPosNrm;
in vec3 vPosSphereNrm;

in float flogz;

uniform float f_coef;

#include <core:shaders/atmosphere/atmo.fsi>

uniform vec3 light_dir;
uniform vec3 camera_pos;

void main()
{
    vec4 atmoc = atmo(light_dir, camera_pos);

    vec3 col = vColor;
   // vec3 col = texture(tex, vTexture).xyz;

    gAlbedoSpec = vec4((col + atmoc.xyz * atmoc.w) * 0.77, 0.0);
    gNormal = vNormal;
    gPosition = vec4(vPos, atmoc.w * 0.5);

    // FragColor = vec4(diff * texture(tex, vTexture).xyz, 1.0);
    // FragColor = vec4(vTexture, 0.0, 1.0);

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}
