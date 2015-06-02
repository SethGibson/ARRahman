#version 150
uniform sampler2D uDepthSampler;
uniform float		uDepthMax;
uniform float		uErrorTerm;
in vec2 UV;

out vec4 FragColor;

float lmap(float val, float inMin, float inMax, float outMin, float outMax)
{
	return outMin + ((outMax - outMin) * (val - inMin)) / (inMax - inMin);
}

void main()
{
	float d = texture(uDepthSampler, UV).x;
	d*=65535;
	vec4 outColor = vec4(0.0,0.0,0.0,1.0);
	
	if(d>0&&d<uDepthMax)
	{
		float c = lmap(d,0.0,uDepthMax,1.0,0.0);
		outColor = vec4(c,c,c,1.0);
	}

	else if (d<=0||d>=uErrorTerm)
		outColor = vec4(0.0,0.0,0.0,1.0);
	FragColor = outColor;
}