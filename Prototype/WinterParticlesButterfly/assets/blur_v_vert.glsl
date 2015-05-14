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

	BlurSamples[0] = ciTexCoord0 + vec2(0.0, -0.014);
    BlurSamples[1] = ciTexCoord0 + vec2(0.0, -0.012);
    BlurSamples[2] = ciTexCoord0 + vec2(0.0, -0.010);
    BlurSamples[3] = ciTexCoord0 + vec2(0.0, -0.008);
    BlurSamples[4] = ciTexCoord0 + vec2(0.0, -0.006);
    BlurSamples[5] = ciTexCoord0 + vec2(0.0, -0.004);
    BlurSamples[6] = ciTexCoord0 + vec2(0.0, -0.002);
    BlurSamples[7] = ciTexCoord0 + vec2(0.0,  0.002);
    BlurSamples[8] = ciTexCoord0 + vec2(0.0,  0.004);
    BlurSamples[9] = ciTexCoord0 + vec2(0.0,  0.006);
    BlurSamples[10] = ciTexCoord0 + vec2(0.0,  0.008);
    BlurSamples[11] = ciTexCoord0 + vec2(0.0,  0.010);
    BlurSamples[12] = ciTexCoord0 + vec2(0.0,  0.012);
    BlurSamples[13] = ciTexCoord0 + vec2(0.0,  0.014);

	/*
	BlurSamples[0] = ciTexCoord0 + vec2(0.0, -0.056);
    BlurSamples[1] = ciTexCoord0 + vec2(0.0, -0.048);
    BlurSamples[2] = ciTexCoord0 + vec2(0.0, -0.040);
    BlurSamples[3] = ciTexCoord0 + vec2(0.0, -0.032);
    BlurSamples[4] = ciTexCoord0 + vec2(0.0, -0.024);
    BlurSamples[5] = ciTexCoord0 + vec2(0.0, -0.016);
    BlurSamples[6] = ciTexCoord0 + vec2(0.0, -0.008);
    BlurSamples[7] = ciTexCoord0 + vec2(0.0,  0.008);
    BlurSamples[8] = ciTexCoord0 + vec2(0.0,  0.016);
    BlurSamples[9] = ciTexCoord0 + vec2(0.0,  0.024);
    BlurSamples[10] = ciTexCoord0 + vec2(0.0,  0.032);
    BlurSamples[11] = ciTexCoord0 + vec2(0.0,  0.040);
    BlurSamples[12] = ciTexCoord0 + vec2(0.0,  0.048);
    BlurSamples[13] = ciTexCoord0 + vec2(0.0,  0.056);

	*/
}