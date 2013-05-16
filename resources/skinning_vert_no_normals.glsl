const int MAXBONES = 92;

attribute vec3 position;
attribute vec2 texcoord;
attribute vec4 boneWeights;
attribute vec4 boneIndices;

uniform bool isAnimated;
uniform mat4 boneMatrices[MAXBONES];
uniform mat4 invTransposeMatrices[MAXBONES];

varying vec2 Tc;
varying vec3 V;

void main()
{	
	vec4 pos = vec4(position, 1.0);
	if( isAnimated ) {
		pos =	boneMatrices[int(boneIndices.x)] * pos * boneWeights.x +
		boneMatrices[int(boneIndices.y)] * pos * boneWeights.y +
		boneMatrices[int(boneIndices.z)] * pos * boneWeights.z +
		boneMatrices[int(boneIndices.w)] * pos * boneWeights.w ;
		
		pos.w = 1.0;
	}
	V = (gl_ModelViewMatrix * pos).xyz;
	Tc = texcoord;
	gl_Position = gl_ModelViewProjectionMatrix * pos;
}