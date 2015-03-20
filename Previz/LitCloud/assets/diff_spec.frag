#version 150

uniform samplerCube uCubeMapTex;
uniform vec3 ViewDirection;
uniform vec3 LightColor;
uniform vec3 LightPosition;

uniform float SpecularScale;
uniform float SpecularPower;
uniform float AmbientScale;
uniform float EnvScale;
uniform float ColorScale;

in vec4 Color;
in vec3 ObjectNormal;
in vec4 WorldPosition;

out vec4 oColor;

void main()
{
/*
	vec3 cNormal = normalize(ObjectNormal);
	vec3 cLightDir = normalize(LightPosition - vec3(WorldPosition.xyz));
	vec3 cEyeDir = normalize(ViewDirection);
	vec3 cReflectDir = reflect(cLightDir, cNormal);
	vec3 cViewReflect = reflect(cEyeDir, cNormal);

	float cDiffTerm = max(dot(cLightDir, cNormal), 0.0);
	float cSpecTerm = pow(max(dot(cEyeDir, cReflectDir), 0), SpecularPower);

	vec3 cReflContrib = texture(uCubeMapTex, cViewReflect).rgb;
	vec3 cAmbientContrib = LightColor * AmbientScale;
	vec3 cSpecContrib = SpecularScale * cSpecTerm * LightColor * Color.rgb;

	vec3 cDayside = Color.rgb*ColorScale*LightColor;
	vec3 cNightside = cReflContrib*EnvScale*LightColor;

	vec3 cFragColor = mix(cNightside, cDayside, cDiffTerm);
	cFragColor += cSpecContrib;
	cFragColor += cAmbientContrib;

	oColor = vec4(cFragColor, 1.0);
	*/
	oColor = Color;
}