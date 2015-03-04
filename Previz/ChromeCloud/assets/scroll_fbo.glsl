#version 150

uniform sampler2D mTexMask;
uniform sampler2D mTexColor;
uniform float ciElapsedSeconds;

in vec2 UV;
out vec4 oColor;

void main()
{
	float cMask = texture2D(mTexMask, vec2(UV.x+ciElapsedSeconds*0.15, UV.y)).r;
	float cColorMask = texture2D(mTexColor, vec2(UV.x+(cMask*0.02), UV.y+(cMask*0.02))).a;
	cMask*=cColorMask;

	vec4 cColor = texture2D(mTexColor, vec2(UV.x+(cMask*0.02), UV.y+(cMask*0.02)));

	oColor = vec4(cColor.rgb, cMask);
}


