#version 150

uniform sampler2D mTexSky;
uniform sampler2D mTexCloud;

in vec2 UV;

out vec4 oColor;

void main()
{
	vec4 cSky = texture2D(mTexSky, UV);
	vec4 cCloud = texture2D(mTexCloud, UV);

	oColor = cSky + cCloud;
}