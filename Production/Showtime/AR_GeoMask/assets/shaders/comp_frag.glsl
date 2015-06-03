#version 150
uniform sampler2D uTextureSamplerRgb;
uniform sampler2D uTextureSamplerMask;
uniform float uRgbScale;

in vec2 UV;

out vec4 FragColor;

void main()
{
	vec4 maskSample = texture(uTextureSamplerMask, UV);
	vec4 depthSample = texture(uTextureSamplerRgb, vec2(1.0-(UV.x+maskSample.r*0.1),UV.y+(maskSample.r*0.1)));
	
	
	vec3 colorTerm = depthSample.rgb*uRgbScale*maskSample.r;

	FragColor = vec4(colorTerm+0.05+(maskSample.g*0.5),maskSample.a);
}