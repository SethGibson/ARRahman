#version 150

uniform sampler2D mTexMask;
uniform sampler2D mTexColor;
uniform float ciElapsedSeconds;

uniform float mCloudSpeed;
uniform float mCloudNoise;

in vec2 UV;
out vec4 oColor;

void main()
{

	float cMask = texture2D(mTexMask, vec2(UV.x+ciElapsedSeconds*mCloudSpeed, UV.y)).a;

	float cMaskAmt = cMask*mCloudNoise;
	float cColorMask = texture2D(mTexColor, vec2(UV.x+cMaskAmt, UV.y+cMaskAmt)).a;

	vec4 cColor = texture2D(mTexColor, vec2(UV.x+cMaskAmt, UV.y+cMaskAmt));
	
	cMask*=cColorMask;
	oColor = vec4(cColor.rgb, cMask);
}


