#version 150
uniform sampler2D	uTextureSampler;

in vec2		UV;

out vec4	FragColor;
void main()
{
	FragColor = texture(uTextureSampler, UV);
}