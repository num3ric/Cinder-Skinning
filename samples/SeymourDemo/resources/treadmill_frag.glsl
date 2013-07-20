uniform sampler2D tex_diffuse;
uniform sampler2D tex_shadow;
uniform float cycle;

varying vec3 L;

void main() {
	vec4 diffuse = texture2D(tex_diffuse, vec2(gl_TexCoord[0].s - cycle, gl_TexCoord[0].t) );
	vec2 center = vec2(gl_TexCoord[0].s + 0.5, gl_TexCoord[0].t );
	vec4 shadow = vec4( texture2D(tex_shadow, center + 0.25 * vec2(L.z, -L.x) ).a );
	gl_FragColor = max(diffuse - shadow, vec4(0,0,0,1));
}