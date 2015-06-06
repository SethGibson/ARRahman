#version 150
uniform vec3 uLightPos = vec3(0,15,-15);
uniform vec3 uEyePos;

in vec3 Normal;
in vec3 FragPos;
in float Alpha;
out vec4 FragColor;

void main()
{
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(uLightPos-FragPos);
	vec3 viewDir = normalize(uEyePos-FragPos);
	vec3 reflDir = reflect(-lightDir, normal);

	float s = pow(max(dot(viewDir,reflDir),0.0), 8.0);
	float d = max(dot(normal,lightDir),0.0);
	FragColor = vec4(vec3(d,s,0.0), Alpha);
}