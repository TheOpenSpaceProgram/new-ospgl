#version 330
out vec4 FragColor;


in vec2 vTex;

uniform sampler2D tex;
uniform vec4 color;

void main()
{
	float val = texture(tex, vTex).a;

	FragColor = color * val;
}
