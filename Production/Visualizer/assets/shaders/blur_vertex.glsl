#version 150
uniform mat4	ciModelViewProjection;
uniform vec2	uBlurAxis;
uniform vec2	uBlurSize;
in vec4			ciPosition;
in vec2			ciTexCoord0;

out vec2		UV;
out vec2		BlurSamples[14];

void main()
{
	gl_Position = ciModelViewProjection * ciPosition;
	UV = ciTexCoord0;

	BlurSamples[0] = ciTexCoord0 + vec2(-0.014*uBlurAxis.x*uBlurSize.x, -0.014*uBlurAxis.y*uBlurSize.y);
    BlurSamples[1] = ciTexCoord0 + vec2(-0.012*uBlurAxis.x*uBlurSize.x, -0.012*uBlurAxis.y*uBlurSize.y);
    BlurSamples[2] = ciTexCoord0 + vec2(-0.010*uBlurAxis.x*uBlurSize.x, -0.010*uBlurAxis.y*uBlurSize.y);
    BlurSamples[3] = ciTexCoord0 + vec2(-0.008*uBlurAxis.x*uBlurSize.x, -0.008*uBlurAxis.y*uBlurSize.y);
    BlurSamples[4] = ciTexCoord0 + vec2(-0.006*uBlurAxis.x*uBlurSize.x, -0.006*uBlurAxis.y*uBlurSize.y);
    BlurSamples[5] = ciTexCoord0 + vec2(-0.004*uBlurAxis.x*uBlurSize.x, -0.004*uBlurAxis.y*uBlurSize.y);
    BlurSamples[6] = ciTexCoord0 + vec2(-0.002*uBlurAxis.x*uBlurSize.x, -0.002*uBlurAxis.y*uBlurSize.y);
    BlurSamples[7] = ciTexCoord0 + vec2(0.002*uBlurAxis.x*uBlurSize.x, 0.002*uBlurAxis.y*uBlurSize.y);
    BlurSamples[8] = ciTexCoord0 + vec2(0.004*uBlurAxis.x*uBlurSize.x, 0.004*uBlurAxis.y*uBlurSize.y);
    BlurSamples[9] = ciTexCoord0 + vec2(0.006*uBlurAxis.x*uBlurSize.x, 0.006*uBlurAxis.y*uBlurSize.y);
    BlurSamples[10] = ciTexCoord0 + vec2(0.008*uBlurAxis.x*uBlurSize.x, 0.008*uBlurAxis.y*uBlurSize.y);
    BlurSamples[11] = ciTexCoord0 + vec2(0.010*uBlurAxis.x*uBlurSize.x, 0.010*uBlurAxis.y*uBlurSize.y);
    BlurSamples[12] = ciTexCoord0 + vec2(0.012*uBlurAxis.x*uBlurSize.x, 0.012*uBlurAxis.y*uBlurSize.y);
    BlurSamples[13] = ciTexCoord0 + vec2(0.014*uBlurAxis.x*uBlurSize.x, 0.014*uBlurAxis.y*uBlurSize.y);
}