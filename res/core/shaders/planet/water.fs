#version 430 core
layout (location = 0) out vec4 gPositionEmit;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gPbr;

out vec4 FragColor;

in vec3 vNormal;
in vec3 vPos;
in float vDepth;
in vec2 vTexture;
in vec3 vPosNrm;

in float flogz;

uniform float time;

uniform float f_coef;
uniform vec3 camera_pos;

#include <core:shaders/atmosphere/atmo.fsi>

uniform vec3 light_dir;
void main()
{
    vec3 atmoc = atmo(light_dir, camera_pos);


    float diff = max(dot(-light_dir, vNormal), 1.0);

    vec3 viewDir = normalize(camera_pos - vNormal);
    vec3 reflectDir = reflect(light_dir, vNormal);

    float spec = 0.0;
    float specular = 1.0 * spec;

    float spec_red = pow(diff, 0.3);

    vec3 veryshallowcol = vec3(0.95, 0.95, 1.0);
    vec3 shallowcol = vec3(0.39, 0.62, 0.72);
    vec3 deepcol = vec3(0.24, 0.43, 0.58) * 0.5;
    vec3 speccol = vec3(1.0, 0.3, 0.3);
    vec3 speccolb = vec3(1.0, 0.9, 0.88);

    float deepfactor = max(min(pow(vDepth, 0.7) * 300.0, 1.0), 0.0);

    vec3 col = shallowcol * (1.0 - deepfactor) + deepcol * deepfactor;

    // Cute wave effect, should be very cheap?
    //float wavefac = (sin((vDepth * 800.0 + sin(time * 0.5) * 0.0008) * 12000.0) + 1.0) * 0.5;
    // Waves only happen on really shallow vertices
    //wavefac *= -min(vDepth * 1000.0 - 0.01, 0) / 0.01;
    //col = 13.0 * wavefac * veryshallowcol + col;

    gAlbedo = (col + atmoc) * 0.77;
    gNormal = vNormal;
    gPbr = vec3(1.0, 0.0, 1.0); // Occlusion, rougness, metallic
    vec3 vPosFixed = vec3(-vPos.x, vPos.y, vPos.z);
    gPositionEmit = vec4(vPosFixed, length(atmoc) + spec);

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}
