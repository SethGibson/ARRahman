#version 150
uniform sampler2D u_SamplerRGB;

in vec2 UV;
out vec4 o_Color;

void main( void )
{
	o_Color = texture2D(u_SamplerRGB, UV);

}