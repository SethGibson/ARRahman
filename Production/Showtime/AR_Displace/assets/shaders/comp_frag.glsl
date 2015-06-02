#version 150
uniform sampler2D	uTextureRgbSampler;
uniform sampler2D	uTextureBloomSampler;
uniform sampler2D	uTextureDepthSampler;
uniform float		uDisplacementAmount;
in vec2		UV;

out vec4	FragColor;
void main()
{
	vec2 imageCenter = vec2(480,270);
	vec2 distFromCenter = normalize(gl_FragCoord.xy-imageCenter);

	float depthSample = texture(uTextureDepthSampler, UV).r;
	
	vec4 colorSample = texture(uTextureRgbSampler, UV+depthSample*uDisplacementAmount*vec2(-distFromCenter.x,0.0));
	vec4 bloomSample = texture(uTextureBloomSampler, UV+depthSample*uDisplacementAmount*vec2(-distFromCenter.x,0.0));
	
	FragColor = colorSample+bloomSample;
}