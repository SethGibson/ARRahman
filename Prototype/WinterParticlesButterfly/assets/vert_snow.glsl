#version 150

uniform mat4 ciViewMatrix;
uniform mat4 ciProjectionMatrix;


in vec4		ciPosition;
in vec2		ciTexCoord0;
in vec3     ciNormal;

in mat4		iModelMatrix;
in vec3		iPosition;
in float	iSize;

out vec2 UV;
out vec3 Normal;
out vec4 WorldPos;

void main()
{
	mat4 cMVP = ciProjectionMatrix * ciViewMatrix * iModelMatrix;
	vec4 cInstPos = (ciPosition*iSize) + vec4(iPosition,1.0);
	
	UV = ciTexCoord0;
	Normal = mat3(transpose(inverse(iModelMatrix)))*ciNormal;
	WorldPos = iModelMatrix * cInstPos;

	gl_Position = cMVP * cInstPos;

}