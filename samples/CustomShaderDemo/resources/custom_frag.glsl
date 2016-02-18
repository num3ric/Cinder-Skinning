#version 150

in vec2 Tc;
in vec3 V, N, L;

uniform sampler2D tex;

out vec4 fragColor;

void main (void)
{
	vec3 E = normalize(-V);
	vec3 R = normalize(-reflect(L, N));
	vec4 Ispec = vec4(1.0) * pow( max( dot(R, E), 0.0 ), 0.5);
	fragColor = vec4(N, 1.0) + Ispec;
}