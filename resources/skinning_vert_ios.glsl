//#version 150

const int MAXBONES = 92;

attribute vec4 ciPosition;
attribute vec3 ciNormal;
attribute vec2 ciTexCoord0;
attribute vec4 ciBoneWeight;
attribute vec4 ciBoneIndex;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

//uniform bool isAnimated;
uniform mat4 boneMats[MAXBONES];
uniform mat4 invTransposeMats[MAXBONES];

// outputs passed to the fragment shader
varying vec4	vVertex;
varying vec3	vNormal;
varying vec2	vTexCoord0;

vec4 skinBone( in vec4 pos ) {
	return	boneMats[int(ciBoneIndex.x)] * pos * ciBoneWeight.x +
			boneMats[int(ciBoneIndex.y)] * pos * ciBoneWeight.y +
			boneMats[int(ciBoneIndex.z)] * pos * ciBoneWeight.z +
			boneMats[int(ciBoneIndex.w)] * pos * ciBoneWeight.w ;
}

vec4 skinInverseTranspose( in vec4 pos ) {
	return	invTransposeMats[int(ciBoneIndex.x)] * pos * ciBoneWeight.x +
			invTransposeMats[int(ciBoneIndex.y)] * pos * ciBoneWeight.y +
			invTransposeMats[int(ciBoneIndex.z)] * pos * ciBoneWeight.z +
			invTransposeMats[int(ciBoneIndex.w)] * pos * ciBoneWeight.w ;
}

void main()
{	
	vec4 pos	= skinBone( ciPosition );
	vec4 norm	= skinInverseTranspose( vec4(ciNormal, 1.0) );
	
	pos.w = 1.0;
	norm.w = 0.0;
	
	vVertex		= ciModelView * pos;
	vNormal		= normalize(ciNormalMatrix * norm.xyz);
	//modulo to support symmetric texture lookups
	vTexCoord0 = mod(ciTexCoord0, vec2(1.0));
	
	gl_Position = ciModelViewProjection * pos;
}
