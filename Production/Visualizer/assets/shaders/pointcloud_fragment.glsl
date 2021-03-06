#version 150
uniform sampler2DRect	uCubemapSampler;
uniform sampler2D		uTextureSampler;
uniform vec3			uLightPosition;
uniform vec3			uViewDirection;
uniform vec3			uTextureMinimum;
uniform vec2			uMovieSize;
uniform float			uSpecularPower;
uniform float			uSpecularStrength;
uniform float			uFresnelPower;
uniform float			uFresnelStrength;
uniform float			uReflectionStrength;
uniform float			uAmbientStrength;

in vec4				FragPos;
in vec3				Normal;
in vec3				LookupVector;
in vec2				UV;

in vec3				ViewNormal;
in vec3				ViewPos;

out vec4			FragColor;
void main()
{
	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(uLightPosition-FragPos.xyz);
	vec3 lightReflect = normalize(reflect(-lightDir, normal));

	vec3 viewDir = normalize(uViewDirection-FragPos.xyz);
	vec3 viewReflect = normalize(reflect(viewDir, normal));
	vec2 viewUV = vec2((viewReflect.x/viewReflect.z+1)*0.5,(viewReflect.y/viewReflect.z+1)*0.5);
	viewUV.x*=uMovieSize.x;
	viewUV.y*=uMovieSize.y;

	vec3 viewNormal = normalize(ViewNormal);
	vec3 viewPos = normalize(-ViewPos);

	vec4 texContrib = texture(uTextureSampler, vec2(UV.x,1-UV.y));
	texContrib.r = max(texContrib.r, uTextureMinimum.x);
	texContrib.g = max(texContrib.g, uTextureMinimum.y);
	texContrib.b = max(texContrib.b, uTextureMinimum.z);

	vec4 ambientContrib = texContrib*uAmbientStrength;

	float diffContrib = max(dot(normal,lightDir), 0.0);
	
	float specContrib = max(pow(dot(viewDir,lightReflect),uSpecularPower),0.0);
	specContrib *= uSpecularStrength;

	FragColor = texContrib*diffContrib+specContrib+ambientContrib;
}