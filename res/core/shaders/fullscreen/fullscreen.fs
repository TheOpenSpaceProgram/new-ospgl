#version 330
out vec4 FragColor;


in vec2 vTex;

uniform sampler2D tex;
uniform int vflip;

void main()
{
    vec4 sample = texture(tex, vec2(vTex.x, 1.0 - vTex.y));
    FragColor = vec4(sample.xyz, 1.0);

}