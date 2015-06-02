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
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(LightPosition-vec3(FragPos.xyz));
	vec3 eyeDir = normalize(ViewDirection);
	vec3 halfV = normalize(lightDir+eyeDir);

	vec3 reflTerm = reflect(lightDir, normal);
	float diffTerm = max(dot(normal,lightDir),0.0);
	float specTerm = pow( max( dot(eyeDir, reflTerm), 0), SpecPow);
	vec4 specColor = Color*specTerm;

	FragColor = Color*diffTerm+specColor*SpecStr;
}