#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

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

const int ATMO_STEPS = 4;
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
uniform vec3 atmo_main_color;
uniform vec3 atmo_sunset_color;
uniform float atmo_exponent;
uniform float sunset_exponent;

uniform vec3 light_dir;

float height(vec3 p)
{
    return max((length(p) - 1.0) / (atmo_radius - 1.0), 0.0);
}

float density(float h)
{
   float SCALE_H = 8.0;
   return min(exp(-pow(h, atmo_exponent) * SCALE_H), 1.0);
}

float atmo_curve(float d)
{
    return 1.1 * (pow(20, d) - 1) / (pow(20, d) + 1);
}



vec2 raySphereIntersect(vec3 r0, vec3 rd, float sr)
{
	float a = dot(rd, rd);
	vec3 s0_r0 = r0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sr * sr);
	float disc = b * b - 4.0 * a* c;

	if (disc < 0.0)
	{
		return vec2(-1.0, -1.0);
	}
	else
	{
		return vec2(-b - sqrt(disc), -b + sqrt(disc)) / (2.0 * a);
	}
}

vec4 atmo(vec3 lightDir)
{
    vec3 start = camera_pos;

    if(length(vPosNrm) > atmo_radius * 1.5)
    {
        return vec4(0.0, 0.0, 0.0, 0.0);
    }

    vec3 ray = vPosNrm - start;

    if(length(start) > atmo_radius * 1.0)
    {
        // Adjust start to intersection with atmosphere
        vec2 intersect = raySphereIntersect(start, ray, atmo_radius * 1.0);
        start = start + intersect.x * ray;
        ray = vPosNrm - start;
    }

    vec3 ipos = start;
    float d = 0.0;

    float step_size = length(ray) / float(ATMO_STEPS - 1);
    float last_d = 0.0;

    float dotp = exp(20.0 * dot(ray, vPosNrm)) * 16.0;

    for(int i = 0; i < ATMO_STEPS; i++)
    {
        float step = float(i) / float(ATMO_STEPS - 1);
        ipos = start + ray * step;

		float h = height(ipos);

		d += density(h) * step_size * (0.25 + dotp);
    }


  	float fade_factor = 0.25;
	  float fade_factor_add = 0.0;

	  float fade = max( min( dot(normalize(vPosNrm), -lightDir) + 0.1, fade_factor), 0.0) * (1.0 / fade_factor) + fade_factor_add;
	  d = min(pow(d, 0.5) * min(fade, 0.5) * 2.0, 1.0);

    float r_color = exp(-sunset_exponent * fade);

    vec3 col = atmo_main_color * (1.0 - r_color) + atmo_sunset_color * r_color;

    return vec4(col, atmo_curve(d));
}

void main()
{
    vec4 atmoc = atmo(light_dir);


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


    vec3 veryshallowcol = vec3(0.95, 0.95, 1.0);
    vec3 shallowcol = vec3(0.39, 0.62, 0.72);
    vec3 deepcol = vec3(0.24, 0.43, 0.58) * 0.5;
    vec3 speccol = vec3(1.0, 0.3, 0.3);
    vec3 speccolb = vec3(1.0, 0.9, 0.88);

    float deepfactor = max(min(pow(vDepth, 0.7) * 300.0, 1.0), 0.0);
    float veryshallow = max(min(pow(vDepth, 0.44 + min(wave * 0.05, 0.0)) * 500.0, 1.0), 0.0);

    vec3 col = shallowcol * (1.0 - deepfactor) + deepcol * deepfactor + veryshallowcol * (1.0 - veryshallow);


    gAlbedoSpec = vec4((col + atmoc.xyz * atmoc.w) * 0.77, 1.0);
    gNormal = nrm;
    gPosition = vPos;

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}