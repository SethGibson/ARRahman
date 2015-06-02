#version 150
uniform mat4 ciViewMatrix;
uniform mat4 ciProjectionMatrix;

in mat4 iModelMatrix;
in vec4 ciPosition;
in vec4 ciNormal;

in float iSize;
in float iAlpha;

out vec3	Normal;
out vec3	FragPos;
out float	Alpha;
void main()
{
	Normal = vec3(mat3(transpose(inverse(iModelMatrix)))*ciNormal.xyz);
	FragPos = vec3(iModelMatrix*ciPosition);
	Alpha = iAlpha;
	gl_Position = ciProjectionMatrix*ciViewMatrix*iModelMatrix*(ciPosition*vec4(iSize,iSize,iSize,1.0));
}