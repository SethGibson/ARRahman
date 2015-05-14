#version 150
uniform sampler2D u_SamplerFBO;

in vec2 UV;
in vec2 BlurSamples[14];

out vec4 FragColor;

void main()
{
	vec4 cAccum = vec4(0);

    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 0])*0.0044299121055113265;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 1])*0.00895781211794;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 2])*0.0215963866053;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 3])*0.0443683338718;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 4])*0.0776744219933;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 5])*0.115876621105;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 6])*0.147308056121;
    cAccum += texture2D(u_SamplerFBO, UV         )*0.159576912161;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 7])*0.147308056121;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 8])*0.115876621105;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[ 9])*0.0776744219933;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[10])*0.0443683338718;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[11])*0.0215963866053;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[12])*0.00895781211794;
    cAccum += texture2D(u_SamplerFBO, BlurSamples[13])*0.0044299121055113265;

	FragColor = vec4(cAccum.rgb*vec3(1.3), cAccum.a);
}