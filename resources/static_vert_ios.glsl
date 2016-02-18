
attribute vec4 ciPosition;
attribute vec3 ciNormal;
attribute vec3 ciTangent;
attribute vec2 ciTexCoord0;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

// outputs passed to the fragment shader
varying vec4	vVertex;
varying vec3	vNormal;
varying vec3	vTangent;
varying vec3	vBiTangent;
varying vec2	vTexCoord0;

void main()
{
	vVertex		= ciModelView * ciPosition;
	vNormal		= normalize(ciNormalMatrix * ciNormal);
	vTangent	= normalize(ciNormalMatrix * ciTangent);
	vBiTangent	= normalize(cross(vNormal, vTangent));
	//modulo to support symmetric texture lookups
	vTexCoord0 = mod(ciTexCoord0, vec2(1.0));
	
	gl_Position = ciModelViewProjection * ciPosition;
}