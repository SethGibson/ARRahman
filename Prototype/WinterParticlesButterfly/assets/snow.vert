#version 150



//uniform mat4	ciModelViewProjection;
uniform mat4 ciViewMatrix;
uniform mat4 ciProjectionMatrix;
//uniform mat3   ciNormalMatrix;


uniform vec3 u_CameraUp;					//camera up vector (from the app)
uniform vec3 u_CameraRight;					//camera right vector (from the app)

in mat4 iModelMatrix;
in vec4		ciPosition;
in vec2		ciTexCoord0;
//in vec3     ciNormal;

in vec3		iPosition;

out vec2 UV;


void main()
{

	UV = ciTexCoord0;
	vec3 mBillboard = iPosition + u_CameraRight*ciPosition.x*4 + u_CameraUp*ciPosition.y*4;

	//gl_Position	= ciModelViewProjection * (4.0*ciPosition + vec4(iPosition,0));
	//TexCoord0	= ciTexCoord0;
	//Normal		= ciNormalMatrix * ciNormal;

	gl_Position = ciProjectionMatrix * ciViewMatrix * iModelMatrix * vec4(mBillboard,1.0);
	//gl_Position = ciModelViewProjection * vec4(mBillboard, 1.0);

}