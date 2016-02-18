#version 150

uniform sampler2D tex;
uniform float transparency;

in vec2 tex_coord;
out vec4 fragColor;

void main() {
	vec4 diffuse = texture(tex, tex_coord.st );
	diffuse.w *= 0.7 * transparency;
	fragColor = diffuse;
}