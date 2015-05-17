#version 150
uniform sampler2D uTextureSampler;

in vec2 UV;

out vec4 FragColor;

void main()
{
	vec4 texSample = texture(uTextureSampler, UV);
	texSample -= 1.0;

	vec4 luminanceColor = -6.0 * texSample * texSample + 2.0;
	float luminance = dot(luminanceColor, vec4(0.3333, 0.3333, 0.3333, 0.0 ));
	
	FragColor = texSample + (luminance+0.6);
}