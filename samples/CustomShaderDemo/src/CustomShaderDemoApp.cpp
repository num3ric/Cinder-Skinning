#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"

#include "Resources.h"

#include "model/AssimpLoader.h"
#include "model/SkeletalMesh.h"
#include "model/Skeleton.h"

using namespace ci;
using namespace ci::app;

class CustomShaderDemo : public App {
public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void fileDrop( FileDropEvent event );
	void keyDown( KeyEvent event );
	void resize();
	void update();
	void draw();
private:
	gl::GlslProgRef			mCustomShader;
	model::SkeletalMeshRef	mSeymour;
	
	CameraPersp				mCamera;
	CameraUi				mCamUi;
	float					mRotationRadius;
	float					mScaleFactor;
	vec3					mLightPos;
};

void CustomShaderDemo::setup()
{
	mScaleFactor = 100.0f;
	mRotationRadius = 20.0f;
	mLightPos = vec3(0, 20.0f, 0);
	try {
		mCustomShader = gl::GlslProg::create( loadResource(RES_CUSTOM_VERT), loadResource(RES_CUSTOM_FRAG) );
	} catch( ci::gl::GlslProgCompileExc &exc ) {
		console() << "Shader compile error: " << std::endl;
		console() << exc.what();
	}
	
	/* Here we pass the custom shader to the skinned vbo mesh so that its
	 * shader attributes (position, normal, texcoord, boneWeights & boneIndices)
	 * are set as the asset is loaded. */
	mSeymour = model::SkeletalMesh::create( model::AssimpLoader( loadResource( RES_SEYMOUR ) ), nullptr, mCustomShader );
	mSeymour->loopAnim( app::timeline() );
}

void CustomShaderDemo::mouseDown( MouseEvent event )
{
	mCamUi.mouseDown( event.getPos() );
}

void CustomShaderDemo::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mCamUi.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void CustomShaderDemo::fileDrop( FileDropEvent event )
{
//	try {
		const auto& loader = model::AssimpLoader( loadFile( event.getFile( 0 ) ) );
		mSeymour = model::SkeletalMesh::create( loader, nullptr, mCustomShader );
//	}
//	catch( ... ) {
//		console() << "unable to load the asset!" << std::endl;
//	};
}

void CustomShaderDemo::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_f ) {
		app::setFullScreen( !isFullScreen() );
	}
}

void CustomShaderDemo::resize()
{
	mCamUi.setWindowSize( getWindowSize() );
	mCamera.setAspectRatio( getWindowAspectRatio() );
	gl::enableDepthRead();
}

void CustomShaderDemo::update()
{
	mLightPos.x = mRotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
	mLightPos.z = mRotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
}

void CustomShaderDemo::draw()
{
	gl::clear( Color::black() );
	gl::setMatrices( mCamera );
	gl::translate(0, -5.0f, 0.0f);
	gl::scale( vec3( mScaleFactor ) );
	
//	gl::drawVector(mLightPos, vec3( 0.0f ) );
	
	/* Custom rendering function similar to SkinnedRenderer::draw( SkeletalMeshRef ).
	 * Furthermore, consult the SkinnedRenderer::drawSection() function to see how to use loaded material info. */
	for( const auto& section : mSeymour->getSections() ) {
		mCustomShader->bind();
		if( section->getSkeleton() ) {
			mCustomShader->uniform( "lightPos", mLightPos );
			mCustomShader->uniform( "boneMatrices", section->getBoneMatrices()->data(), model::SkeletalMesh::MAXBONES );
			mCustomShader->uniform( "invTransposeMatrices", section->getInvTransposeMatrices()->data(), model::SkeletalMesh::MAXBONES );
		}
		section->getBatch()->draw();
	}
}

CINDER_APP( CustomShaderDemo, RendererGl )
