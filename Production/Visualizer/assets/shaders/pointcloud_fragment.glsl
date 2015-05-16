#version 150
uniform samplerCube	uCubemapSampler;
uniform sampler2D	uTextureSampler;
uniform vec3		uLightPosition;
uniform vec3		uViewDirection;
uniform float		uSpecularPower;
uniform float		uSpecularStrength;
uniform float		uFresnelPower;
uniform float		uFresnelStrength;
uniform float		uReflectionStrength;

in vec4				FragPos;
in vec3				Normal;
in vec3				LookupVector;
in vec2				UV;

out vec4			FragColor;
void main()
{
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(uLightPosition-FragPos.xyz);
	vec3 viewDir = normalize(uViewDirection);
	vec3 lightReflect = normalize(reflect(-lightDir, normal));
	vec3 viewReflect = normalize(reflect(viewDir, normal));

	vec4 reflContrib = texture(uCubemapSampler, viewReflect);
	reflContrib *= uReflectionStrength;
	vec4 texContrib = texture(uTextureSampler, vec2(UV.x,1-UV.y));
	
	float diffContrib = max(dot(normal,lightDir), 0.0);
	
	float specContrib = max(pow(dot(viewDir,lightReflect),uSpecularPower),0.0);
	specContrib *= uSpecularStrength;
	
	float fresContrib = 1.0 - (max(pow(dot(normal,viewDir),uFresnelPower),0.0));
	fresContrib *= uFresnelStrength;
	
	FragColor = texContrib*diffContrib+specContrib+(reflContrib*fresContrib);
}