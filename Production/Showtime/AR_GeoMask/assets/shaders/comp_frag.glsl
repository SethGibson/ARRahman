#version 150
uniform sampler2D uTextureSamplerRgb;
uniform sampler2D uTextureSamplerMask;
uniform float uRgbScale;

in vec2 UV;

out vec4 FragColor;

void main()
{
	vec4 depthSample = texture(uTextureSamplerRgb, UV);
	vec4 maskSample = texture(uTextureSamplerMask, UV);
	
	vec3 colorTerm = depthSample.rgb*uRgbScale*maskSample.r;

	FragColor = vec4(colorTerm+0.05+(maskSample.g*0.5),maskSample.a);
}