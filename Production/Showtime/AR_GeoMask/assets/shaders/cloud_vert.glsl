#version 150
uniform mat4 ciModelViewProjection;
uniform mat4 ciModelMatrix;
in vec4 ciPosition;
in vec2	ciTexCoord0;

out vec2 UV;
out vec4 FragPos;

void main()
{
	FragPos = ciModelMatrix*ciPosition;
	UV = ciTexCoord0;
	gl_Position = ciModelViewProjection*ciPosition;
}