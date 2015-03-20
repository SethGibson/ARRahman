#version 150

uniform float ciElapsedSeconds;

uniform sampler2D mTexDisplace;
uniform sampler2D mTexColor;
uniform sampler2D mTexFire;

uniform float DisplaceAmount;
uniform float ScrollSpeed;

in vec2 UV;

out vec4 oColor;

void main()
{
	/*
	float cScrollAmt = ciElapsedSeconds*ScrollSpeed;
	float cDispSample = texture2D(mTexDisplace, vec2(UV.x, UV.y-(ciElapsedSeconds*ScrollSpeed))).a;
	float cDispAmt = cDispSample*DisplaceAmount;

	vec2 cUV = vec2(UV.x, UV.y-cDispAmt);
	vec4 cColor = texture2D(mTexFire, cUV);
	
	oColor = vec4(cColor.rgb, cDispSample*cColor.a);
	*/

	float cDispSample = texture2D(mTexDisplace, vec2(UV.x, UV.y-(ciElapsedSeconds*ScrollSpeed))).a;
	float cDispAmt = cDispSample*DisplaceAmount;
	vec2 cUV = vec2(UV.x, UV.y-cDispAmt);

	vec4 cRgbColor = texture2D(mTexColor, cUV);
	vec4 cFireColor = texture2D(mTexFire, vec2(1.0-cRgbColor.r));
	oColor = vec4(cFireColor.rgb, cRgbColor.a*0.25);
}