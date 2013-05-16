#ifdef GL_ES
precision highp float;
#endif

varying vec2 Tc;
varying vec3 V;

uniform sampler2D texture;

void main (void)
{
	vec4 Dm = texture2D( texture, Tc );
	
	vec3 L = normalize(gl_LightSource[0].position.xyz);
	vec3 E = normalize(-V);
	
	vec4 Iamb = gl_FrontLightProduct[0].ambient * Dm;
	vec4 Idiff = gl_FrontLightProduct[0].diffuse * Dm;
		
	gl_FragColor = gl_FrontMaterial.emission + Iamb + Idiff;
}