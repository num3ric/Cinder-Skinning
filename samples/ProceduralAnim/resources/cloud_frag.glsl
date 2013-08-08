uniform sampler2D tex;
uniform float transparency;

void main() {
	vec4 diffuse = texture2D(tex, gl_TexCoord[0].st );
	diffuse.w *= 0.7 * transparency;
	gl_FragColor = diffuse;
}