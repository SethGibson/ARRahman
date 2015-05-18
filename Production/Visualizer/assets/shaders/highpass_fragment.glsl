#version 150
uniform sampler2D	uTextureSampler;
uniform float		uLuminance;
uniform float		uWhiteCenter;
uniform float		uThreshold;

in vec2 UV;

out vec4 FragColor;
/*
void main()
{
	vec4 texSample = texture(uTextureSampler, UV);
	texSample *= (uWhiteCenter/uLuminance);
	texSample *= 1.0 + (texSample /(uThreshold*uThreshold));
	texSample -= 0.5;
	texSample /= (1.0+texSample);
	
	FragColor = texSample;
}

*/
void main()
{
    vec4 tex;
    vec4 bright4;
    float bright;
    
    vec2 texCoord = UV;

    tex = texture( uTextureSampler, texCoord);
    tex -= 1.00000;
    bright4 = -6.00000 * tex * tex + 2.00000;
    bright = dot( bright4, vec4( 0.333333, 0.333333, 0.333333, 0.000000) );
    tex += (bright + 0.600000);

    gl_FragColor = tex;
}