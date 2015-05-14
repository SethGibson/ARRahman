#version 150
uniform mat4	ciModelViewProjection;
in vec4			ciPosition;
in vec2			ciTexCoord0;

out vec2		UV;
out vec2		BlurSamples[14];

void main()
{
	gl_Position = ciModelViewProjection * ciPosition;
	UV = ciTexCoord0;

	BlurSamples[0] = ciTexCoord0 + vec2(-0.014, 0.0);
    BlurSamples[1] = ciTexCoord0 + vec2(-0.012, 0.0);
    BlurSamples[2] = ciTexCoord0 + vec2(-0.010, 0.0);
    BlurSamples[3] = ciTexCoord0 + vec2(-0.008, 0.0);
    BlurSamples[4] = ciTexCoord0 + vec2(-0.006, 0.0);
    BlurSamples[5] = ciTexCoord0 + vec2(-0.004, 0.0);
    BlurSamples[6] = ciTexCoord0 + vec2(-0.002, 0.0);
    BlurSamples[7] = ciTexCoord0 + vec2( 0.002, 0.0);
    BlurSamples[8] = ciTexCoord0 + vec2( 0.004, 0.0);
    BlurSamples[9] = ciTexCoord0 + vec2( 0.006, 0.0);
    BlurSamples[10] = ciTexCoord0 + vec2( 0.008, 0.0);
    BlurSamples[11] = ciTexCoord0 + vec2( 0.010, 0.0);
    BlurSamples[12] = ciTexCoord0 + vec2( 0.012, 0.0);
    BlurSamples[13] = ciTexCoord0 + vec2( 0.014, 0.0);

	/*
	BlurSamples[0] = ciTexCoord0 + vec2(-0.056, 0.0);
    BlurSamples[1] = ciTexCoord0 + vec2(-0.048, 0.0);
    BlurSamples[2] = ciTexCoord0 + vec2(-0.040, 0.0);
    BlurSamples[3] = ciTexCoord0 + vec2(-0.032, 0.0);
    BlurSamples[4] = ciTexCoord0 + vec2(-0.024, 0.0);
    BlurSamples[5] = ciTexCoord0 + vec2(-0.016, 0.0);
    BlurSamples[6] = ciTexCoord0 + vec2(-0.008, 0.0);
    BlurSamples[7] = ciTexCoord0 + vec2( 0.008, 0.0);
    BlurSamples[8] = ciTexCoord0 + vec2( 0.016, 0.0);
    BlurSamples[9] = ciTexCoord0 + vec2( 0.024, 0.0);
    BlurSamples[10] = ciTexCoord0 + vec2( 0.032, 0.0);
    BlurSamples[11] = ciTexCoord0 + vec2( 0.040, 0.0);
    BlurSamples[12] = ciTexCoord0 + vec2( 0.048, 0.0);
    BlurSamples[13] = ciTexCoord0 + vec2( 0.056, 0.0);
	*/
}