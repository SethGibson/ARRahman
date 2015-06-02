#version 150
uniform mat4	ciModelViewProjection;

in vec4			ciPosition;
in vec4			iPosition;
in vec3			iColor;
in float		iSize;

out vec3 Color;

void main()
{
	Color = iColor;
	vec4 scaled = vec4(ciPosition.x*iSize,ciPosition.y*iSize,ciPosition.z*iSize, 1.0);
	gl_Position = ciModelViewProjection*(scaled+iPosition);
}