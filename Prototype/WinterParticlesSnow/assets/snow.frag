#version 150

uniform samplerCube mTexCube;

in vec3	NormalWorldSpace;
in vec3 EyeDirWorldSpace;


out vec4 	oColor;

void main( void )
{
	// reflect the eye vector about the surface normal (all in world space)
	vec3 refractedEyeWorldSpace = refract( EyeDirWorldSpace, normalize(NormalWorldSpace), 0.8f );
	oColor = texture( mTexCube, refractedEyeWorldSpace );
	//oColor = vec4(1.0f,1.0f,1.0f,1.0f);
}