#version 150

uniform samplerCube uCubeMapTex;

in vec3	CubeLUVector;
in vec3 EyeDirWorldSpace;
in vec3 NormalWorldSpace;
in vec3 Color;

out vec4 	oColor;

void main( void )
{
	vec3 reflectedEyeWorldSpace = reflect( EyeDirWorldSpace, normalize(CubeLUVector) );

	vec4 tColor = texture( uCubeMapTex, reflectedEyeWorldSpace );
	//vec4 tColor = texture( uCubeMapTex, CubeLUVector );
	
	vec4 cColor = vec4(Color,1.0);

	oColor = cColor + (tColor*0.5);
}