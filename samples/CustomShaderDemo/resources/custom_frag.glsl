#ifdef GL_ES
precision highp float;
#endif

varying vec2 Tc;
varying vec3 V, N, L;

void main (void)
{
	vec4 Dm = vec4(N, 1.0);
	
	vec3 L = normalize(gl_LightSource[0].position.xyz);
	vec3 E = normalize(-V);
	vec3 R = normalize(reflect(L, N));
	vec4 Ispec = vec4(1.0) * pow( max( dot(R, E), 0.0 ), 2.0 );
		
	gl_FragColor = vec4(N, 1.0) + Ispec;
}