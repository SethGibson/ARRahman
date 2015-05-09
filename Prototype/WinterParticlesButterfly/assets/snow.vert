#version 150

uniform mat4 ciViewMatrix;
uniform mat4 ciProjectionMatrix;


in vec4		ciPosition;
in vec2		ciTexCoord0;
in vec3     ciNormal;

in mat4		iModelMatrix;
in vec3		iPosition;

out vec2 UV;

void main()
{
	UV = ciTexCoord0;
	
	mat4 cMVP = ciProjectionMatrix * ciViewMatrix * iModelMatrix;
	gl_Position = cMVP * ( (ciPosition*vec4(2.5,2.5,2.5,1)) + vec4(iPosition,1.0));

}