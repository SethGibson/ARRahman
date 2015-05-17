#version 150
uniform mat4	ciViewMatrix;
uniform mat4	ciProjectionMatrix;

in vec4			ciPosition;
in vec3			ciNormal;
in vec2			ciTexCoord0;

in mat4			iModelMatrix;
in vec3			iPosition;
in float		iSize;

out vec4		FragPos;
out vec3		Normal;
out vec2		UV;

void main()
{
	vec3 finalPosition = ciPosition.xyz*iSize+iPosition;
	mat4 normalMatrix = mat4(mat3(ciViewMatrix*iModelMatrix));

	FragPos = ciViewMatrix*iModelMatrix*vec4(finalPosition,1.0);
	Normal = vec3(normalMatrix*vec4(ciNormal,0.0));
	UV = ciTexCoord0;

	gl_Position = ciProjectionMatrix*ciViewMatrix*iModelMatrix*vec4(finalPosition,1.0);
}