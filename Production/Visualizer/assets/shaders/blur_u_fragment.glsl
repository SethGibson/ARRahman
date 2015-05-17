#version 150
uniform sampler2D uTextureSampler;
uniform vec2 uImageSize;

uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
uniform float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

in vec2 UV;

out vec4 FragColor;

void main(void)
{
	vec4 blurSample = texture( uTextureSampler, (vec2(gl_FragCoord)/uImageSize)) * weight[0];
    
	for (int i=1; i<3; i++)
	{
        blurSample += texture( uTextureSampler, ((vec2(gl_FragCoord)+vec2(offset[i],0.0))/uImageSize)) * weight[i];
        blurSample += texture( uTextureSampler, ((vec2(gl_FragCoord)-vec2(offset[i],0.0))/uImageSize)) * weight[i];
    }

	FragColor = blurSample;
}