#version 150
uniform mat4 ciViewMatrix;
uniform mat4 ciProjectionMatrix;

in vec3		ciNormal;
in vec4		ciPosition;

in mat4		iModelMatrix;
in float	iSize;
in vec4		iColor;

out vec4 Color;
out vec3 Normal;
out vec4 FragPos;

void main()
{
	vec3 position = ciPosition.xyz*iSize;

	Color = iColor;
	Normal = mat3(transpose(inverse(iModelMatrix)))*ciNormal;
	FragPos = iModelMatrix*vec4(position,1.0);

	gl_Position = ciProjectionMatrix*ciViewMatrix*iModelMatrix*vec4(position,1.0);
}