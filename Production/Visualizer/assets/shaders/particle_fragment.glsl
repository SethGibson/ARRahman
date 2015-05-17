#version 150
uniform sampler2D	uTextureSampler;
uniform vec3		uLightPosition;
uniform vec3		uViewDirection;
uniform float		uSpecularPower;
uniform float		uSpecularStrength;
uniform float		uAmbientStrength;

in vec4				FragPos;
in vec3				Normal;
in vec2				UV;

out vec4			FragColor;

void main()
{
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(uLightPosition-FragPos.xyz);
	vec3 viewDir = normalize(uViewDirection);
	vec3 lightReflect = normalize(reflect(-lightDir, normal));

	vec4 texContrib = texture(uTextureSampler, UV);
	float diffContrib = max(dot(normal,lightDir), 0.0);
	vec4 ambContrib = texContrib * uAmbientStrength;
	float specContrib = max(pow(dot(viewDir,lightReflect),uSpecularPower),0.0);
	specContrib *= uSpecularStrength;
	vec3 specColor = texContrib.rgb*specContrib;
	
	
	FragColor = mix(texContrib+vec4(specColor,1.0), ambContrib, diffContrib);
}