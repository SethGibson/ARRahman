#version 150
uniform sampler2D uTextureSampler;

in vec2 UV;
in vec4 FragPos;
out vec4 FragColor;

void main()
{
	vec4 texColor = texture(uTextureSampler, vec2(UV.x,1.0-UV.y));
	FragColor = vec4(texColor.rgb+texColor.rgb,1.0);
}