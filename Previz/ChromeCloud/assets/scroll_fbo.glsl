#version 150

uniform sampler2D mTexMask;
uniform sampler2D mTexColor;
uniform float ciElapsedSeconds;

in vec2 UV;
out vec4 oColor;

void main()
{
	float cNoise = noise1(10);
	float cMask = texture2D(mTexMask, vec2(UV.x+ciElapsedSeconds*0.15, UV.y)).a;
	float cColorMask = texture2D(mTexColor, vec2(UV.x+(cMask*cNoise), UV.y)).a;

	vec4 cColor = texture2D(mTexColor, vec2(UV.x+(cMask*cNoise), UV.y));
	
	cMask*=cColorMask;
	oColor = vec4(cColor.rgb, cMask);
}


