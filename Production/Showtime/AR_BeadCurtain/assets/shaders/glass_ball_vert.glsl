#version 150
uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;
uniform mat4 ciViewMatrix;

in vec4 ciPosition;
in vec4 ciNormal;

in vec3 iPosition;
in float iSize;

out vec4 Position;
out vec4 Normal;
void main()
{
	vec3 position = ciPosition.xyz*vec3(iSize);
	Normal = mat4(mat3(transpose(inverse(ciModelMatrix))))*ciNormal;
	Position = ciModelMatrix*vec4(position,1.0);
	gl_Position = ciModelViewProjection*vec4(position+iPosition,1.0);
}