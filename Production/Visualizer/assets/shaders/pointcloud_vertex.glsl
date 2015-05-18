#version 150
uniform mat4	ciModelMatrix;
uniform mat4	ciViewMatrix;
uniform mat4	ciProjectionMatrix;

in vec4			ciPosition;
in vec3			ciNormal;

in vec3			iPosition;
in vec2			iTexCoord0;
in float		iSize;

out vec4		FragPos;
out vec3		Normal;
out vec3		LookupVector;
out vec2		UV;

out vec3		ViewNormal;
out vec3		ViewPos;

void main()
{
	vec3 finalPosition = ciPosition.xyz*iSize+iPosition;
	mat4 normalMatrix = mat4(mat3(ciViewMatrix*ciModelMatrix));

	FragPos = ciViewMatrix*ciModelMatrix*vec4(finalPosition,1.0);
	Normal = vec3(normalMatrix*vec4(ciNormal,0.0));
	LookupVector = vec3(ciModelMatrix*vec4(finalPosition,1.0));
	UV = iTexCoord0;

	ViewNormal = mat3(ciViewMatrix)*ciNormal;
	ViewPos = vec3(ciViewMatrix*vec4(finalPosition,1.0));

	gl_Position = ciProjectionMatrix*ciViewMatrix*ciModelMatrix*vec4(finalPosition,1.0);
}