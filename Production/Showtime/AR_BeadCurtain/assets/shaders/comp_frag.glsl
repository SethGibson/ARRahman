#version 150
uniform sampler2D	uTextureRgbSampler;
uniform sampler2D	uTextureBloomSampler;
in vec2		UV;

out vec4	FragColor;

vec3 screen(vec3 baseColor, vec3 screenColor)
{
	return ( 1.0- (1.0 - baseColor) * (1.0 - screenColor) );
}

void main()
{
	vec4 color = texture(uTextureRgbSampler, UV);
	vec4 bloom = texture(uTextureBloomSampler, UV);

	//vec3 screened = screen(colorSample.rgb, bloomSample.rgb);

	FragColor = vec4(color+bloom);
}

