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

	BlurSamples[0] = ciTexCoord0 + vec2(0.0, -0.028);
    BlurSamples[1] = ciTexCoord0 + vec2(0.0, -0.024);
    BlurSamples[2] = ciTexCoord0 + vec2(0.0, -0.020);
    BlurSamples[3] = ciTexCoord0 + vec2(0.0, -0.016);
    BlurSamples[4] = ciTexCoord0 + vec2(0.0, -0.012);
    BlurSamples[5] = ciTexCoord0 + vec2(0.0, -0.008);
    BlurSamples[6] = ciTexCoord0 + vec2(0.0, -0.004);
    BlurSamples[7] = ciTexCoord0 + vec2(0.0,  0.004);
    BlurSamples[8] = ciTexCoord0 + vec2(0.0,  0.008);
    BlurSamples[9] = ciTexCoord0 + vec2(0.0,  0.012);
    BlurSamples[10] = ciTexCoord0 + vec2(0.0,  0.016);
    BlurSamples[11] = ciTexCoord0 + vec2(0.0,  0.020);
    BlurSamples[12] = ciTexCoord0 + vec2(0.0,  0.024);
    BlurSamples[13] = ciTexCoord0 + vec2(0.0,  0.028);
}