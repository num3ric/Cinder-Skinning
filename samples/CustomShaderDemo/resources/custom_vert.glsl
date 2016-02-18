

#version 150

const int MAXBONES = 92;

in vec4 ciPosition;
in vec3 ciNormal;
in vec2 ciTexCoord0;
in vec4 ciBoneWeight;
in vec4 ciBoneIndex;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

uniform vec3 lightPos;
uniform mat4 boneMatrices[MAXBONES];
uniform mat4 invTransposeMatrices[MAXBONES];

out vec2 Tc;
out vec3 V, N, L;

void main()
{
	vec4 pos = ciPosition;
	vec4 norm = vec4(ciNormal, 1.0);
	pos = boneMatrices[int(ciBoneIndex.x)] * pos * ciBoneWeight.x +
	boneMatrices[int(ciBoneIndex.y)] * pos * ciBoneWeight.y +
	boneMatrices[int(ciBoneIndex.z)] * pos * ciBoneWeight.z +
	boneMatrices[int(ciBoneIndex.w)] * pos * ciBoneWeight.w ;
	
	norm = invTransposeMatrices[int(ciBoneIndex.x)] * norm * ciBoneWeight.x +
	invTransposeMatrices[int(ciBoneIndex.y)] * norm * ciBoneWeight.y +
	invTransposeMatrices[int(ciBoneIndex.z)] * norm * ciBoneWeight.z +
	invTransposeMatrices[int(ciBoneIndex.w)] * norm * ciBoneWeight.w ;
	pos.w = 1.0;
	norm.w = 0.0;

	V = (ciModelView * pos).xyz;
	N = -normalize(ciNormalMatrix * norm.xyz);
	L = normalize(lightPos);
	Tc = ciTexCoord0;
	gl_Position = ciModelViewProjection * pos;
}