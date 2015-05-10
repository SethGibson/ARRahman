#version 150
uniform mat4	ciProjectionMatrix;
uniform mat4	u_ViewMatrix;
in vec4			ciPosition;

out highp vec3	NormalWorldSpace;

void main( void )
{
	mat4 cViewMatrix = mat4(mat3(u_ViewMatrix));
	NormalWorldSpace = vec3( ciPosition );
	gl_Position = ciProjectionMatrix * cViewMatrix * vec4(ciPosition.xyz,1.0);
	gl_Position = gl_Position.xyww;
}
