#version 150
uniform mat4		ciProjectionMatrix;
uniform mat4		ciModelMatrix;
uniform mat4		ciViewMatrix;

in vec4				ciPosition;
out vec3			LookupVector; 

void main()
{
	LookupVector = vec3(ciModelMatrix * ciPosition);

	mat4 skyboxMatrix = mat4(mat3(ciViewMatrix));
	gl_Position = ciProjectionMatrix*skyboxMatrix*ciPosition;
	gl_Position = gl_Position.xyww;
}