#version 150
uniform sampler2D	uTextureSampler;
uniform float		uBlurResolution;
uniform float		uBlurRadius;
uniform vec2		uBlurAxis;

in vec2				UV;

out vec4			FragColor;

void main()
{
    vec4 sum = vec4(0.0);

    float blurSize = uBlurRadius/uBlurResolution; 

    float uStep = uBlurAxis.x;
    float vStep = uBlurAxis.y;

    sum += texture(uTextureSampler, vec2(UV.x - 4.0*blurSize*uStep, UV.y - 4.0*blurSize*vStep)) * 0.0162162162;
    sum += texture(uTextureSampler, vec2(UV.x - 3.0*blurSize*uStep, UV.y - 3.0*blurSize*vStep)) * 0.0540540541;
    sum += texture(uTextureSampler, vec2(UV.x - 2.0*blurSize*uStep, UV.y - 2.0*blurSize*vStep)) * 0.1216216216;
    sum += texture(uTextureSampler, vec2(UV.x - 1.0*blurSize*uStep, UV.y - 1.0*blurSize*vStep)) * 0.1945945946;

    sum += texture(uTextureSampler, UV) * 0.2270270270;

    sum += texture(uTextureSampler, vec2(UV.x + 1.0*blurSize*uStep, UV.y + 1.0*blurSize*vStep)) * 0.1945945946;
    sum += texture(uTextureSampler, vec2(UV.x + 2.0*blurSize*uStep, UV.y + 2.0*blurSize*vStep)) * 0.1216216216;
    sum += texture(uTextureSampler, vec2(UV.x + 3.0*blurSize*uStep, UV.y + 3.0*blurSize*vStep)) * 0.0540540541;
    sum += texture(uTextureSampler, vec2(UV.x + 4.0*blurSize*uStep, UV.y + 4.0*blurSize*vStep)) * 0.0162162162;

	FragColor = sum;
}