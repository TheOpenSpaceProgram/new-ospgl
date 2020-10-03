#version 330
out vec4 FragColor;


in vec2 vTex;

uniform sampler2D tex;
uniform int vflip;

void main()
{
    vec2 coord = vTex;

    if(vflip != 0)
    {
        coord = vec2(vTex.x, 1.0 - vTex.y);
    }

    const float gamma = 2.2;
    const float exposure = 1.0f;

    vec3 color = texture(tex, coord).rgb;
    vec3 mapped = vec3(1.0) - exp(-color * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);

}
