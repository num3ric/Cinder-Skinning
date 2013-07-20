
varying vec3 L;

void main()
{
	L = normalize( vec3( gl_ModelViewMatrixInverse * gl_LightSource[0].position ) );
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}