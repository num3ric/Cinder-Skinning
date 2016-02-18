#version 150

in vec4 ciPosition;
in vec2 ciTexCoord0;
uniform mat4 ciModelViewProjection;

out vec2 tex_coord;

void main()
{
	tex_coord = ciTexCoord0;
	gl_Position = ciModelViewProjection * ciPosition;
}