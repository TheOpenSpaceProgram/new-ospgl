#version 330 core

out vec4 FragColor;

in vec3 vNormal;
in vec3 vPos;
in float vDepth;
in vec2 vTexture;

in float flogz;

uniform float time;

uniform float f_coef;
uniform vec3 camera_pos;

const int ATMO_STEPS = 3;
const float ATMO_STEPS_INVERSE = 1.0 / float(ATMO_STEPS);

uniform bool do_atmo;


// Simplex 2D noise
//
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

uniform float atmo_radius;

float height(vec3 p)
{
    return max((length(p) - 1.0) / (atmo_radius - 1.0), 0.0);
}

float density(float h)
{
   float SCALE_H = 8.0 / (1.0);
   return min(exp2(-h * SCALE_H), 1.0);
}


vec4 atmo(vec3 lightDir)
{
    vec3 start = camera_pos;

    if(length(camera_pos) > atmo_radius * 10.0)
    {
        start = normalize(camera_pos) * atmo_radius * 10.0;
    }

    vec3 ray = vPos - start;

    vec3 ipos = start;
    float d = 0.0;

    float step_size = length(ray) / float(ATMO_STEPS);
    float last_d = 0.0;
    for(int i = 0; i < ATMO_STEPS; i++)
    {
      float step = float(i) / float(ATMO_STEPS - 1);
      ipos = start + ray * step;

      float h = height(ipos);

      d += density(h) * ATMO_STEPS_INVERSE * step_size;
    }


    float fade_factor = 0.25;
    float fade_factor_add = 0.0;

    float fade = max( min( dot(vPos, -lightDir) + 0.1, fade_factor), 0.0) * (1.0 / fade_factor) + fade_factor_add;
    d = min(pow(d, 0.23) * min(fade, 1.0) * 1.6, 0.7);
    vec3 col = vec3(0.6, 0.7, 1.5);

    return vec4(col, d);
}

void main()
{
    float wave_power = max(min(0.0000001 / pow(length(camera_pos - vPos), 2.0), 1.0), 0.0);

    float wave = snoise(vTexture * 703071.0 + time * 0.4);
    float wave2 = snoise(vTexture * 703071.0 * 1.25 - time * 0.1);
    float wavedir = sin(time * 0.2);
    float wavedir2 = cos(time * 0.2);
    // Distortion vectors for the normal
    vec3 da = cross(vNormal, vec3(0.0, 1.0, 0.0));
    vec3 db = cross(vNormal, da);

    vec3 offset = (da * wave * 0.1 * wavedir + db * wave2 * 0.1 * wavedir2) * wave_power;

    vec3 nrm = normalize(vNormal + offset);


    vec3 lightDir = normalize(vec3(-0.4, -1.0, -0.4));
    float diff = max(dot(-lightDir, vNormal + offset * 0.5), 0.0);

    vec3 viewDir = normalize(camera_pos - vPos);
    vec3 reflectDir = reflect(lightDir, nrm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
    float specular = 1.0 * spec;

    float spec_red = pow(diff, 0.3);

    vec3 veryshallowcol = vec3(0.95, 0.95, 1.0);
    vec3 shallowcol = vec3(0.7, 0.7, 1.0);
    vec3 deepcol = vec3(0.6, 0.6, 0.9);
    vec3 speccol = vec3(1.0, 0.3, 0.3);
    vec3 speccolb = vec3(1.0, 0.9, 0.88);

    float deepfactor = max(min(pow(vDepth, 0.88) * 500.0, 1.0), 0.0);
    float veryshallow = max(min(pow(vDepth, 0.44 + min(wave * 0.05, 0.0)) * 500.0, 1.0), 0.0);

    vec3 col = shallowcol * (1.0 - deepfactor) + deepcol * deepfactor + veryshallowcol * (1.0 - veryshallow);

    vec4 atmoc = atmo(lightDir);

    FragColor = vec4(col * diff + speccol * specular * (1.0 - spec_red) + speccolb * specular * spec_red + atmoc.xyz * atmoc.w, min(deepfactor + 0.9, 1.0));

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}