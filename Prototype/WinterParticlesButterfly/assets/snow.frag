#version 150
uniform sampler2D	u_SamplerRGB;
uniform vec3		u_ViewDir;
uniform vec3		u_LightPos;

in vec2 UV;
in vec3 Normal;
in vec4 WorldPos;

out vec4 o_Color;

void main( void )
{
	vec3 cNormal = normalize(Normal);
	vec3 cLightDir = normalize(u_LightPos - WorldPos.xyz);
	vec3 cEyeDir = normalize(u_ViewDir);

	float cDiffTerm = max(dot(cNormal, cLightDir), 0.0);
	float cFrenTerm = 1.0 - max( pow (dot(cNormal, -cEyeDir), 64.0), 0.0);
	cFrenTerm  *= 0.25;
	vec4 cTexTerm = texture2D(u_SamplerRGB, UV);
	o_Color = vec4(cTexTerm.rgb+vec3(cFrenTerm), 1.0);
}