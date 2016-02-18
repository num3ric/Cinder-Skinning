#version 330

const int MAXBONES = 92;

in vec4 ciPosition;
in vec3 ciNormal;
in vec3 ciTangent;
in vec2 ciTexCoord0;
in vec4 ciBoneWeight;
in vec4 ciBoneIndex;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

//uniform bool isAnimated;
uniform mat4 boneMats[MAXBONES];
uniform mat4 invTransposeMats[MAXBONES];

// outputs passed to the fragment shader
out vec4	vVertex;
out vec3	vNormal;
out vec3	vTangent;
out vec3	vBiTangent;
out vec2	vTexCoord0;

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
	vec4 tang	= skinBone( vec4(ciTangent, 1.0) );
	vec4 norm	= skinInverseTranspose( vec4(ciNormal, 1.0) );
	
	pos.w = 1.0;
	norm.w = 0.0;
	
	vVertex		= ciModelView * pos;
	vNormal		= normalize(ciNormalMatrix * norm.xyz);
	vTangent	= normalize(ciNormalMatrix * tang.xyz);
	vBiTangent	= normalize(cross(vNormal, vTangent));
	//modulo to support symmetric texture lookups
	vTexCoord0 = mod(ciTexCoord0, vec2(1.0));
	
	gl_Position = ciModelViewProjection * pos;
}