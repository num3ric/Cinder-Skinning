#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/Light.h"

#include "ModelSourceAssimp.h"
#include "SkinnedVboMesh.h"
#include "Resources.h"
#include "Skeleton.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace model;

class CustomShaderDemo : public AppNative {
public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
	void resize();
	void update();
	void draw();
private:
	gl::GlslProgRef		mCustomShader;
	SkinnedVboMeshRef	mSeymour;
	
	MayaCamUI						mMayaCam;
	float							rotationRadius;
	Vec3f							mLightPos;
};

void CustomShaderDemo::setup()
{
	rotationRadius = 20.0f;
	mLightPos = Vec3f(0, 20.0f, 0);
	try {
		mCustomShader = gl::GlslProg::create( loadResource(RES_CUSTOM_VERT), loadResource(RES_CUSTOM_FRAG) );
	} catch( ci::gl::GlslProgCompileExc &exc ) {
		console() << "Shader compile error: " << std::endl;
		console() << exc.what();
	}
	
	/* Here we pass the custom shader to the skinned vbo mesh so that its
	 * shader attributes (position, normal, texcoord, boneWeights & boneIndices)
	 * are set as the asset is loaded. */
	mSeymour = SkinnedVboMesh::create( loadModel( loadResource( RES_SEYMOUR ) ), nullptr, mCustomShader );
	mSeymour->getSkeleton()->loopAnim( app::timeline() );
}

void CustomShaderDemo::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void CustomShaderDemo::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void CustomShaderDemo::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_f ) {
		app::setFullScreen( !isFullScreen() );
	}
}

void CustomShaderDemo::resize()
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
	gl::enableDepthRead();
}

void CustomShaderDemo::update()
{
	// Always update the mesh if the skeleton is animated.
	mSeymour->update();
	mLightPos.x = rotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
	mLightPos.z = rotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
}

void CustomShaderDemo::draw()
{
	gl::clear( Color::black() );
	gl::setMatrices( mMayaCam.getCamera() );
	gl::translate(0, -5.0f, 0.0f);
	
	gl::Light light( gl::Light::DIRECTIONAL, 0 );
	light.setAmbient( Color::white() );
	light.setDiffuse( Color::white() );
	light.setSpecular( Color::white() );
	light.lookAt( mLightPos, Vec3f::zero() );
	light.update( mMayaCam.getCamera() );
	light.enable();
	
	
	gl::drawVector(mLightPos, Vec3f::zero() );
	
	gl::enable( GL_LIGHTING );
	gl::enable( GL_NORMALIZE );
	
	
	/* Custom rendering function similar to SkinnedRenderer::draw( SkinnedVboMeshRef ).
	 * Furthermore, consult the SkinnedRenderer::drawSection() function to see how to use loaded material info. */
	for( const SkinnedVboMesh::MeshVboSectionRef& section : mSeymour->getSections() ) {
		mCustomShader->bind();
		if( section->hasSkeleton() ) {
			mCustomShader->uniform( "boneMatrices", section->mBoneMatricesPtr->data(), SkinnedVboMesh::MAXBONES );
			mCustomShader->uniform( "invTransposeMatrices", section->mInvTransposeMatricesPtr->data(), SkinnedVboMesh::MAXBONES );
		}
		ci::gl::draw( section->getVboMesh() );
		mCustomShader->unbind();
	}
}

CINDER_APP_NATIVE( CustomShaderDemo, RendererGl )
