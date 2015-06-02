#version 150
uniform samplerCube	uCubemapSampler;
uniform vec3		uEyePos;
uniform vec3		uLightPos;
uniform float		uRefraction;

in vec4 Position;
in vec4 Normal;
in vec3 Color;

out vec4 FragColor;

void main()
{
	vec3 normal = normalize(Normal.xyz);
	vec3 position = Position.xyz;
	
	//specular
	vec3 viewDir = normalize(uEyePos-position);
	vec3 lightDir = normalize(uLightPos-position);
	vec3 reflectDir = reflect(-lightDir, normal);
	float specContrib = pow( max(dot(viewDir, reflectDir), 0.0), 8.0f);
	
	//refraction
    vec3 R = refract(-viewDir, normalize(Normal.xyz), uRefraction);
    vec4 refractContrib = texture(uCubemapSampler, R);

	FragColor = vec4(refractContrib.rgb+(specContrib+Color)+Color,1.0);
}  