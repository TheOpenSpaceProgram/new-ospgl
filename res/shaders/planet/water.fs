#version 330 core

out vec4 FragColor;
  
in vec3 vNormal;
in vec3 vPos;
in float vDepth;

in float flogz;


uniform float f_coef;
uniform vec3 camera_pos;


void main()
{

    vec3 lightDir = normalize(vec3(-0.4, -1.0, -0.4));
    float diff = max(dot(-lightDir, vNormal), 0.1);

    vec3 viewDir = normalize(camera_pos - vPos);
    vec3 reflectDir = reflect(lightDir, vNormal); 

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    float specular = 1.0 * spec; 

    vec3 shallowcol = vec3(0.7, 0.7, 1.0);
    vec3 deepcol = vec3(0.5, 0.5, 0.9);

    float deepfactor = max(min(pow(vDepth, 0.8) * 1000.0, 1.0), 0.0);

    vec3 col = shallowcol * (1.0 - deepfactor) + deepcol * deepfactor;

    FragColor = vec4(col * (diff + specular), min(deepfactor + 0.5, 1.0));

    // Could be removed for that sweet optimization, but some
    // clipping can happen on weird planets
    gl_FragDepth = log2(flogz) * f_coef * 0.5;
}