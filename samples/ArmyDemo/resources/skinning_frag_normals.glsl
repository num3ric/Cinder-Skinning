#ifdef GL_ES
precision highp float;
#endif

varying vec2 Tc;
varying vec3 N, L, V;

uniform sampler2D texture;

void main (void)
{
	vec4 Dm = texture2D( texture, Tc );
	
	vec3 L = normalize(gl_LightSource[0].position.xyz);
	vec3 E = normalize(-V);
	vec3 R = normalize(reflect(L, N));
	
	vec4 Iamb = gl_FrontLightProduct[0].ambient * Dm;
	
	vec4 Idiff = gl_FrontLightProduct[0].diffuse * max(dot(N, L), 0.0) * Dm;
		
//	vec4 Ispec =  gl_FrontLightProduct[0].specular * pow( max( dot(R, E), 0.0 ), 0.95 );
	vec4 Ispec = 0.2*gl_LightSource[0].specular * pow( max( dot(R, E), 0.0 ), 0.95 );
	
//	gl_FragColor = gl_FrontMaterial.emission + Iamb + Idiff + Ispec;
	gl_FragColor = gl_FrontMaterial.emission + 2.0*(Iamb + Idiff + Ispec);
}