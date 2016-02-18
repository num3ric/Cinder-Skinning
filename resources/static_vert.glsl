#version 330

in vec4 ciPosition;
in vec3 ciNormal;
in vec3 ciTangent;
in vec2 ciTexCoord0;

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

// outputs passed to the fragment shader
out vec4	vVertex;
out vec3	vNormal;
out vec3	vTangent;
out vec3	vBiTangent;
out vec2	vTexCoord0;

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