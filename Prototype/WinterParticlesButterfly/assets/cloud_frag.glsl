#version 150

uniform sampler2D mTexRgb;
uniform samplerCube mTexCube;
uniform vec3 ViewDirection;
uniform vec3 LightPosition;

in vec2 UV;
in vec3 ObjectNormal;
in vec4 WorldPosition;

out vec4 oColor;

void main()
{
	vec3 cNormal = normalize(ObjectNormal);
	vec3 cLightDir = normalize(LightPosition-vec3(WorldPosition.xyz));
	vec3 cEyeDir = normalize(ViewDirection);

	float cDiffTerm = max(dot(cNormal,cLightDir), 0.0);
	vec4 cCubeContrib = texture(mTexCube, cNormal);
	vec4 cTexContrib = texture2D(mTexRgb, vec2(UV.x,1.0-UV.y));
	//oColor = mix(cTexContrib, cCubeContrib, cDiffTerm);
	oColor = vec4(cTexContrib.rgb*vec3(1.8), cTexContrib.a);
}