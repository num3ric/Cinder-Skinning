

#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"
//#include "cinder/gl/TextureFont.h"
#include "cinder/Camera.h"
#include "cinder/app/App.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/Context.h"
#include "cinder/Text.h"
#include "cinder/Surface.h"

#include "Renderer.h"
#include "Resources.h"
#include "SkeletalTriMesh.h"
#include "Skeleton.h"
#include "Node.h"
#include "SkeletalMesh.h"

using namespace ci;
using namespace model;

std::unique_ptr<Renderer> Renderer::mInstance = nullptr;

std::once_flag Renderer::mOnceFlag;

Light::Light()
: position(10.0f, 5.0f, 10.0f)
, diffuse( ColorA::white() )
, specular( ColorA::white() )
{ }

LightRef Renderer::mLight = Light::create();

Renderer& Renderer::instance()
{
	std::call_once(mOnceFlag,
				   [] {
					   mInstance.reset( new Renderer );
				   });
	return *mInstance.get();
}

Renderer::Renderer()
{
	try {
		mShaders[ MeshType::STATIC ]	= gl::GlslProg::create( app::loadResource( RES_STATIC_VERT ), app::loadResource( RES_MODEL_FRAG ) );
		mShaders[ MeshType::SKELETAL ]	= gl::GlslProg::create( app::loadResource( RES_SKINNING_VERT ), app::loadResource( RES_MODEL_FRAG ) );
		mShaders[ MeshType::MORPHED ]	= gl::GlslProg::create( app::loadResource( RES_MORPH_VERT ), app::loadResource( RES_MODEL_FRAG ) );
	}
	catch( gl::GlslProgCompileExc &exc ) {
		app::console() << "Shader compile error: " << std::endl;
		app::console() << exc.what();
	}
}

void Renderer::bindGlobalUniforms( const ci::gl::GlslProgRef& shader )
{
	shader->uniform( "uDiffuseMap", 0 );
	shader->uniform( "uLight.position", gl::getModelView() *  vec4( mLight->position, 1 ) );
	shader->uniform( "uLight.diffuse", mLight->diffuse );
	shader->uniform( "uLight.specular", mLight->specular );
}

void Renderer::bindSectionUniforms( const std::shared_ptr<ABatchSection>& section, const ci::gl::GlslProgRef& shader )
{
	shader->uniform( "uMaterial.ambient", section->getMaterial().getAmbient() );
	shader->uniform( "uMaterial.diffuse", section->getMaterial().getDiffuse() );
	shader->uniform( "uMaterial.specular", section->getMaterial().getSpecular() );
	shader->uniform( "uMaterial.emission", section->getMaterial().getEmission() );
	shader->uniform( "uMaterial.shininess", section->getMaterial().getShininess() );
}

void Renderer::draw( const StaticMeshRef& mesh, int sectionId )
{
	auto& shader = instance().mShaders.at( MeshType::STATIC );
	gl::ScopedGlslProg prog( shader );
	bindGlobalUniforms( shader );
	
	drawSections( mesh->getSections(), shader, sectionId );
}

void Renderer::draw( const SkeletalTriMeshRef& mesh, int sectionId )
{
	for( const SkeletalTriMesh::SectionRef& section : mesh->getSections() ) {
		auto drawMesh = [section] {
			gl::Batch::create( *section->getTriMesh(),
							   gl::getStockShader( gl::ShaderDef().color().texture() ) )->draw();
		};
		drawSection( *section.get(), drawMesh);
	}
}

void Renderer::draw( const SkeletalMeshRef& mesh, int sectionId )
{
	auto& shader = instance().mShaders.at( MeshType::SKELETAL );
	gl::ScopedGlslProg prog( shader );
	bindGlobalUniforms( shader );
	shader->uniform( "boneMats", mesh->getBoneMatrices().data(), SkeletalMesh::MAXBONES );
	shader->uniform( "invTransposeMats", mesh->getInvTransposeMatrices().data(), SkeletalMesh::MAXBONES );
	 
	drawSections( mesh->getSections(), shader, sectionId );
}

void Renderer::draw( const MorphedMeshRef& mesh, int sectionId )
{
	
	auto& shader = instance().mShaders.at( MeshType::MORPHED );
	gl::ScopedGlslProg prog( shader );
	bindGlobalUniforms( shader );
	shader->uniform( "uWeights", mesh->getMorphTargetWeights().data(), MorphedMesh::MAXMORPHTARGETS );
	
	drawSections( mesh->getSections(), shader, sectionId );
}

void Renderer::draw( const SkeletonRef& skeleton, const std::string& rootNodeName )
{
	const NodeRef& root = ( !rootNodeName.empty() && skeleton->hasBone( rootNodeName ) ) ? skeleton->getBones().at(rootNodeName) : skeleton->getRootNode();
	
	gl::ScopedGlslProg bind( gl::getStockShader( gl::ShaderDef().color() ) );
#if ! defined( CINDER_GL_ES )
	bool wireframe = gl::isWireframeEnabled();
	gl::enableWireframe();
#endif
	instance().drawAbsolute( skeleton, root );
	instance().drawRelative( skeleton, root );
#if ! defined( CINDER_GL_ES )
	gl::setWireframeEnabled( wireframe );
#endif
}

void Renderer::drawLabels( std::shared_ptr<Skeleton> skeleton, const CameraPersp& camera )
{
	gl::ScopedState	depth( GL_DEPTH_TEST, false );
	gl::ScopedBlend alpha( false );
	gl::ScopedMatrices push;
	
	const auto& modelMat = gl::getModelMatrix();
	gl::setMatricesWindow( app::getWindowSize() );
	skeleton->traverseNodes( [&] ( NodeRef n ) {
		if( skeleton->isNodeVisible( n ) ) {
			vec2 pos = camera.worldToScreen( vec3( modelMat *  vec4( n->getAbsolutePosition(), 1 ) ), app::getWindowWidth()
											, app::getWindowHeight() );
			gl::drawStringCentered( n->getName(), pos );
		}
	} );
}

void Renderer::drawSection( const AMeshSection& section, std::function<void()> drawMesh )
{
	if( section.hasDefaultTransformation() ) {
		gl::pushModelMatrix();
		gl::multModelMatrix( section.getDefaultTranformation() );
	}
	
	if ( section.getTexture() ) {
		section.getTexture()->bind();
	}
	gl::enable( GL_CULL_FACE );
	
	drawMesh();
	
	// Texture Binding
	if(  section.getTexture() ) {
		section.getTexture()->unbind();
	}
	
	if( section.hasDefaultTransformation() ) {
		gl::popModelMatrix();
	}
	gl::disable( GL_CULL_FACE );
}

void Renderer::drawRelative( SkeletonRef skeleton, const NodeRef& node )
{
	gl::ScopedMatrices push{};
	gl::multModelMatrix( node->getRelativeTransformation() );
	if( skeleton->isNodeVisible( node ) ) {
		float scale = 0.2f * length( node->getRelativePosition() );
		gl::VertBatch vb(GL_LINES);
		vb.color(1,0,0);
		vb.vertex( vec3( 0 ) );
		vb.color(1,0,0);
		vb.vertex( vec3( scale, 0, 0 ) );
		vb.color(0,1,0);
		vb.vertex( vec3( 0 ) );
		vb.color(0,1,0);
		vb.vertex( vec3( 0, scale, 0 ) );
		vb.color(0,0,1);
		vb.vertex( vec3( 0 ) );
		vb.color(0,0,1);
		vb.vertex( vec3( 0, 0, scale ) );
		vb.draw();
	}
	for( NodeRef child : node->getChildren() ) {
		drawRelative( skeleton, child );
	}
}

void Renderer::drawAbsolute( SkeletonRef skeleton, const NodeRef& node )
{
	skeleton->traverseNodes( [skeleton] ( NodeRef n ) {
		if( skeleton->isNodeVisible( n ) ) {
			std::shared_ptr<Node> parent( n->getParent().lock() );
			if( parent ) {
				vec3 currentPos = n->getAbsolutePosition();
				gl::color( n->isAnimated() ? Color(1.0f, 0.0f, 0.0f) : Color(0.0f, 1.0f, 0.0f) );
				drawBone( currentPos, parent->getAbsolutePosition() );
			}
		}
	} );
}

void Renderer::drawBone( const vec3& start, const vec3& end )
{
	using namespace ci::gl;
	float dist = distance( start, end );
	auto ctx = context();
	auto shader = gl::getStockShader( gl::ShaderDef().color() );
	gl::ScopedGlslProg bind( shader );
	
	gl::pushModelMatrix();
	gl::translate(start);
	gl::drawSphere( vec3( 0 ), 0.1f * dist , 4);
	gl::popModelMatrix();
	
	color( Color::white() );
	
	float maxGirth = 0.07f * dist;
	const int NUM_SEGMENTS = 4;
	const int numVertices = NUM_SEGMENTS + 2;
	
	GLfloat data[numVertices*3];
	vec3 *boneVerts = (vec3*)data;
	
	vec3 axis = normalize( start - end );
	vec3 temp = ( dot( axis, vec3( 0, 1, 0 ) ) > 0.999f ) ? cross( axis, vec3( 1, 0, 0 ) ) : cross( axis, vec3( 0, 1, 0 ) );
	vec3 left = 0.5f * normalize( cross( axis, temp ) );
	vec3 up = 0.5f * normalize( cross( axis, left ) );
	boneVerts[0] = vec3( end + axis * dist );
	boneVerts[1] = vec3( end + maxGirth * ( axis + left ) );
	boneVerts[2] = vec3( end + maxGirth * ( axis + up ) );
	boneVerts[3] = vec3( end + maxGirth * ( axis - left ) );
	boneVerts[4] = vec3( end + maxGirth * ( axis - up ) );
	boneVerts[5] = vec3( end + maxGirth * ( axis + left ) );
	
	ctx->pushVao();
	ctx->getDefaultVao()->replacementBindBegin();
	
	VboRef defaultVbo = ctx->getDefaultArrayVbo( sizeof(float)*numVertices*3 );
	defaultVbo->bind();
	int posLoc = shader->getAttribSemanticLocation( geom::Attrib::POSITION );
	if( posLoc >= 0 ) {
		enableVertexAttribArray( posLoc );
		vertexAttribPointer( posLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
		defaultVbo->bufferSubData( 0, sizeof(float)*numVertices*3, data );
	}
	
	ctx->getDefaultVao()->replacementBindEnd();
	ctx->setDefaultShaderVars();
	ctx->drawArrays( GL_TRIANGLE_FAN, 0, NUM_SEGMENTS+2 );
	defaultVbo->unbind();
	ctx->popVao();
}
