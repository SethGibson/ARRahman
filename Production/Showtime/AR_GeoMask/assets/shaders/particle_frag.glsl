#version 150
uniform vec3 uLightPos = vec3(0,0,10);

in vec3 Normal;
in vec3 FragPos;
in float Alpha;
out vec4 FragColor;

void main()
{
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(uLightPos-FragPos);

	float d = max(dot(normal,lightDir),0.0);
	FragColor = vec4(vec3(1)*d, Alpha);
}