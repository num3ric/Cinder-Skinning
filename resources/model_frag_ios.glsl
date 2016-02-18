#ifdef GL_ES
precision highp float;
#endif

// light source information
struct LightSource
{
	vec4 position;
	vec4 diffuse;
	vec4 specular;
};

uniform LightSource	uLight;

struct Material
{
	vec4	ambient;
	vec4	diffuse;
	vec4	specular;
	vec4	emission;
	float	shininess;
};

uniform Material uMaterial;

// inputs passed from the vertex shader
varying vec4				vVertex;
varying vec3				vNormal;
varying vec2				vTexCoord0;

uniform sampler2D	uDiffuseMap;

//varying highp vec4 fragColor;

void main (void)
{
	// modify it using the normal & tangents from the 3D mesh (normal mapping)
	vec3	surfaceNormal	= vNormal;
	
	// apply each of our light sources
	vec4 texColor			= texture2D( uDiffuseMap, vTexCoord0 );
	vec4 diffuseColor		= uMaterial.emission + uMaterial.ambient * texColor;
	//	diffuseColor.a			= 1.0f;
	vec4 specularColor		= vec4( 0, 0, 0, 1 );
	vec3 toCamera			= normalize( -vVertex.xyz );
	
	// calculate view space light vectors
	vec3	toLight = normalize( uLight.position.xyz - vVertex.xyz );
	vec3	reflectDir = normalize( -reflect( toLight, surfaceNormal ) );
	
	// calculate diffuse term
	float	fDiffuse = max( dot( surfaceNormal, toLight ), 0.0 );
	fDiffuse = clamp( fDiffuse, 0.1, 1.0 );
	
	// calculate specular term
	float	fSpecular = pow( max( dot( reflectDir, toCamera ), 0.0 ), uMaterial.shininess );
	fSpecular = clamp( fSpecular, 0.0, 1.0 );
	
	// calculate final colors
	diffuseColor += uMaterial.diffuse * texColor * uLight.diffuse * fDiffuse;
	specularColor += uMaterial.specular * uLight.specular * fSpecular;
	
//	// output colors to buffer
	gl_FragColor.rgb = (diffuseColor + specularColor).rgb;
	gl_FragColor.a = diffuseColor.a;
}
