#version 150
uniform samplerCube uSamplerCube;

in vec3 LookupVector;

out vec4 FragColor;

void main()
{
	FragColor = texture(uSamplerCube, LookupVector);
}