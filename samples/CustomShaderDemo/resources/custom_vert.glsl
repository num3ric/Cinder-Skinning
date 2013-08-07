const int MAXBONES = 92;

attribute vec3 position;
attribute vec3 normal;
attribute vec2 texcoord;
attribute vec4 boneWeights;
attribute vec4 boneIndices;

uniform mat4 boneMatrices[MAXBONES];
uniform mat4 invTransposeMatrices[MAXBONES];

varying vec2 Tc;
varying vec3 V, N, L;

void main()
{	
	vec4 pos = vec4(position, 1.0);
	vec4 norm = vec4(normal, 1.0);
	
	pos = boneMatrices[int(boneIndices.x)] * pos * boneWeights.x +
		  boneMatrices[int(boneIndices.y)] * pos * boneWeights.y +
		  boneMatrices[int(boneIndices.z)] * pos * boneWeights.z +
		  boneMatrices[int(boneIndices.w)] * pos * boneWeights.w ;
	
	norm = invTransposeMatrices[int(boneIndices.x)] * norm * boneWeights.x +
		   invTransposeMatrices[int(boneIndices.y)] * norm * boneWeights.y +
		   invTransposeMatrices[int(boneIndices.z)] * norm * boneWeights.z +
		   invTransposeMatrices[int(boneIndices.w)] * norm * boneWeights.w ;
	
	pos.w = 1.0;
	norm.w = 0.0;
	
	V = (gl_ModelViewMatrix * pos).xyz;
	N = normalize(gl_NormalMatrix * norm.xyz);
	L = normalize(vec3(gl_LightSource[0].position));
	Tc = texcoord;
	
	gl_Position = gl_ModelViewProjectionMatrix * pos;
}