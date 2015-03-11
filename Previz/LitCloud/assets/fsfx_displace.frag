#version 150

uniform float ciElapsedSeconds;

uniform sampler2D mTexDisplace;
uniform sampler2D mTexColor;

uniform float DisplaceAmount;
uniform float ScrollSpeed;

in vec2 UV;

out vec4 oColor;

void main()
{
	float cScrollAmt = ciElapsedSeconds*ScrollSpeed;
	float cDispSample = texture2D(mTexDisplace, vec2(UV.x, UV.y-(ciElapsedSeconds*ScrollSpeed))).a;
	float cDispAmt = cDispSample*DisplaceAmount;

	vec2 cUV = vec2(UV.x, UV.y-cDispAmt);
	vec4 cColor = texture2D(mTexColor, cUV);

	oColor = vec4(cColor.rgb, cDispSample*cColor.a);
}