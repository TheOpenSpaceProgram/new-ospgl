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


void main()
{
   
    float wave = snoise(vTexture * 1030701.0 + time * 0.3);
    float wave2 = snoise(vTexture * 705020.0 + 500.0 + time * 0.4);
    float wavedirs = snoise(vTexture * 54326.2 + sin(time * 0.2));
    float wavedir = sin(time * wavedirs);
    float wavedir2 = cos(time * wavedirs * wavedir);
    // Distortion vectors for the normal
    vec3 da = cross(vNormal, vec3(0.0, 1.0, 0.0));
    vec3 db = cross(vNormal, da);

    vec3 nrm = normalize(vNormal + da * wave * 0.1 * wavedir + db * wave2 * 0.1 * wavedir2);
  

    vec3 lightDir = normalize(vec3(-0.4, -1.0, -0.4));
    float diff = max(dot(-lightDir, vNormal), 0.0);

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

    FragColor = vec4(col * diff + speccol * specular * (1.0 - spec_red) + speccolb * specular * spec_red, min(deepfactor + 0.9, 1.0));

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}