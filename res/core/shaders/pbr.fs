#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 vPos;
in vec3 vNrm;
in vec2 vTex;
in mat3 TBN;
in vec3 vTgt;

in float flogz;
uniform float f_coef;

uniform sampler2D base_color_tex;
uniform sampler2D metallic_roughness_tex;
uniform sampler2D ambient_occlusion_tex;
uniform sampler2D normal_map;
uniform sampler2D emissive_tex;

uniform vec3 base_color;
uniform float metallic;
uniform float roughness;
uniform vec3 emissive;
uniform float normal_scale;
uniform float occlusion_strength;
uniform float transparency;

uniform int drawable_id;

#include <core:/shaders/screen_door.fsi>

void main()
{
	screen_door_transparency(transparency);

    vec3 normal = texture(normal_map, vTex).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(TBN * normal);
    normal = vNrm;

    gPosition = vec4(vPos, 0.0f);
    gNormal = normalize(normal);
    //gAlbedoSpec.rgb = texture(base_color_tex, vTex).rgb;
    gAlbedoSpec.rgb = vTgt;
    gAlbedoSpec.a = texture(metallic_roughness_tex, vTex).b;

   	gl_FragDepth = log2(flogz) * f_coef * 0.5;
}

