#version 330
out vec4 FragColor;


in vec2 vTex;

uniform sampler2D tex;
uniform int vflip;

void main()
{
    if(vflip != 0)
    {
        FragColor = texture(tex, vec2(vTex.x, 1.0 - vTex.y));
    }
    else
    {
        FragColor = texture(tex, vTex);
    }

}