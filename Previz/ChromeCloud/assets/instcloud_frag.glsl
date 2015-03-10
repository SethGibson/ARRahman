#version 150

uniform samplerCube uCubeMapTex;

uniform float mColorAmt;
uniform float mReflAmt;

in vec3	NormalWorldSpace;
in vec3 EyeDirWorldSpace;
in vec3	Color;

out vec4 	oColor;

void main( void )
{
	// reflect the eye vector about the surface normal (all in world space)
	vec3 reflectedEyeWorldSpace = reflect( EyeDirWorldSpace, normalize(NormalWorldSpace) );

	vec4 cEnvColor = texture( uCubeMapTex, reflectedEyeWorldSpace );
	vec4 cRgbColor = vec4(Color,1);
	
	oColor = vec4(Color*mColorAmt,1.0)+(cEnvColor*mReflAmt);
}