#version 150
uniform sampler2D	uTextureSampler;
uniform float		uDesaturation;
uniform float		uBrightness;
in vec2 UV;

out vec4 FragColor;

void main()
{
	vec4 sample = texture(uTextureSampler, UV);
	vec3 grayscale = vec3(0.3,0.59,0.11);
	float grayfactor = dot(grayscale, sample.rgb);

	FragColor = vec4(mix(sample.rgb, vec3(grayfactor), uDesaturation )*uBrightness, sample.a );

}