#version 150
uniform sampler2D	uTextureRgbSampler;
uniform sampler2D	uTextureBloomSampler;
uniform sampler2D	uTextureDepthSampler;
uniform float		uDisplacementAmount;
in vec2		UV;

out vec4	FragColor;
void main()
{
	vec2 imageCenter = vec2(480,270);
	vec2 distFromCenter = normalize(gl_FragCoord.xy-imageCenter);

	float depthSample = texture(uTextureDepthSampler, UV).r;
	
	vec4 colorSample = texture(uTextureRgbSampler, UV+depthSample*uDisplacementAmount*vec2(-distFromCenter.x,0.0));
	vec4 bloomSample = texture(uTextureBloomSampler, UV+depthSample*uDisplacementAmount*vec2(-distFromCenter.x,0.0));
	
	FragColor = colorSample+bloomSample;
}

/*
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float uDisplacementAmount = (iMouse.x / iResolution.x - 0.5);
    vec2 uvs = fragCoord.xy/iResolution.xy;
    vec4 depthSample = texture2D(iChannel1,uvs);
    
    vec2 imageCenter = iResolution.xy*0.5;
    vec2 distFromCenter = normalize(fragCoord.xy-imageCenter);
    
    vec2 normDist = distFromCenter/(iResolution.xy*0.5);
    
    fragColor = texture2D(iChannel0, uvs+ vec2(disp.b*(amt*distFromCenter)));
}*/