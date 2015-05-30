#version 150
uniform vec3 ViewDirection;
uniform vec3 LightPosition;
uniform float SpecPow;
uniform float SpecStr;

in vec3 Normal;
in vec4 FragPos;
in vec4 Color;

out vec4 FragColor;

void main()
{
	vec3 cNormal = normalize(Normal);
	vec3 cLightDir = normalize(LightPosition-vec3(FragPos.xyz));
	vec3 cEyeDir = normalize(ViewDirection);
	vec3 cHalfV = normalize(cLightDir+cEyeDir);

	vec3 cRefl = reflect(cLightDir, cNormal);
	float diffTerm = max(dot(cNormal,cLightDir),0.0);
	float cSpecTerm = pow( max( dot(cEyeDir, cRefl), 0), SpecPow);

	FragColor = Color*diffTerm+cSpecTerm*SpecStr;
}