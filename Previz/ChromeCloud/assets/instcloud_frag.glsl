#version 150

uniform samplerCube uCubeMapTex;

in vec3	NormalWorldSpace;
in vec3 EyeDirWorldSpace;
in vec3	Color;

out vec4 	oColor;

void main( void )
{
	// reflect the eye vector about the surface normal (all in world space)
	vec3 reflectedEyeWorldSpace = reflect( EyeDirWorldSpace, normalize(NormalWorldSpace) );

	vec4 cEnvColor = texture( uCubeMapTex, reflectedEyeWorldSpace );
	//vec4 cEnvColor = texture( uCubeMapTex, NormalWorldSpace );
	vec4 cRgbColor = vec4(Color,1);
	oColor = cRgbColor + (cEnvColor*0.95);
}