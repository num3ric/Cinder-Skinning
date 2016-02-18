attribute vec4 ciPosition;
attribute vec3 ciNormal;
attribute vec3 ciTangent;
attribute vec2 ciTexCoord0;

// These attributes need to be manually added/updated if more morph targets are added
attribute vec3 vPositionDiff0;
attribute vec3 vPositionDiff1;
attribute vec3 vPositionDiff2;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;
uniform float ciElapsedSeconds;

uniform float uWeights[10];

varying vec4	vVertex;
varying vec3	vNormal;
varying vec3	vTangent;
varying vec3	vBiTangent;
varying vec2	vTexCoord0;

void main()
{	
	vec4 pos	= ciPosition;
	pos.xyz		+= uWeights[0] * vPositionDiff0;
	pos.xyz		+= uWeights[1] * vPositionDiff1;
	pos.xyz		+= uWeights[2] * vPositionDiff2;

	vec4 tang	= vec4(ciTangent, 1.0);
	vec4 norm	= vec4(ciNormal, 1.0);
	
	pos.w = 1.0;
	
	vVertex		= ciModelView * pos;
	vNormal		= normalize(ciNormalMatrix * norm.xyz);
	vTangent	= normalize(ciNormalMatrix * tang.xyz);
	vBiTangent	= normalize(cross(vNormal, vTangent));
	//modulo to support symmetric texture lookups
	vTexCoord0 = mod(ciTexCoord0, vec2(1.0));
	
	gl_Position = ciModelViewProjection * pos;
}