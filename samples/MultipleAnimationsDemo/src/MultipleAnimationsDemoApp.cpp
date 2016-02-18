/* README: This Ogre-xml model has 15 different animation channels.
   These will serve as testcases for the development of the block's animation classes/API.
   And yes, the first animation is glitchy at the moment...
 */

#include "model/AssimpLoader.h"

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"


#include "model/SkeletalMesh.h"
#include "model/Renderer.h"

using namespace model;

class MultipleAnimationsDemo : public App {
public:
	void setup();
	
	void resize();
	void keyDown( KeyEvent event );
	void mouseMove( MouseEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void mouseWheel( MouseEvent event );
	void fileDrop( FileDropEvent event );
	
	void update();
	void draw();
private:
	void playAnim();
	void loopAnim();
	void stopAnim();
	
	SkeletalMeshRef					mActor;
	float							mScaleFactor;
	
	CameraPersp						mCamera;
	CameraUi						mCamUi;
	float							mMouseHorizontalPos;
	float							mRotationRadius;
	
	float							mFps;
	params::InterfaceGl				mParams;
	bool mDrawSkeleton, mDrawLabels, mDrawMesh, mDrawRelative, mEnableWireframe;
	int mAnimId;
};

void MultipleAnimationsDemo::playAnim()
{
	mActor->playAnim( app::timeline(), mAnimId );
}

void MultipleAnimationsDemo::loopAnim()
{
	mActor->loopAnim( app::timeline(), mAnimId );
}

void MultipleAnimationsDemo::stopAnim()
{
	mActor->stop();
}

void MultipleAnimationsDemo::setup()
{
	model::Skeleton::sRenderMode = model::Skeleton::RenderMode::CLEANED;
	
	mScaleFactor = 1.0f;
	mRotationRadius = 20.0f;
	mMouseHorizontalPos = 0;
	mParams = params::InterfaceGl( "Parameters", ivec2( 200, 250 ) );
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addSeparator();
	mDrawMesh = true;
	mParams.addParam( "Draw Mesh", &mDrawMesh );
	mDrawSkeleton = false;
	mParams.addParam( "Draw Skeleton", &mDrawSkeleton );
	mDrawLabels = false;
	mParams.addParam( "Draw Labels", &mDrawLabels );
	mEnableWireframe = false;
	mParams.addParam( "Wireframe", &mEnableWireframe );
	mParams.addSeparator();
	mAnimId = 0;
	mParams.addParam( "Cycle Id", &mAnimId ).min( 0 ).max( 14 );
	mParams.addButton( "Play Anim", std::bind( &MultipleAnimationsDemo::playAnim, this) );
	mParams.addButton( "Loop Anim", std::bind( &MultipleAnimationsDemo::loopAnim, this) );
	mParams.addButton( "Stop Anim", std::bind( &MultipleAnimationsDemo::stopAnim, this) );
	
	mActor = SkeletalMesh::create( AssimpLoader( loadAsset( "Sinbad.mesh.xml" ) ) );
	
	
	// Blend both animations with a factor of one since they affect independent bones (lower vs upper body).
	// mSkeletalTriMesh->getSkeleton()->loopAnim( { {0, 1.0f}, {7, 1.0f} } ); // this beautiful C++11 syntax is not supported on older version of vc11
	auto weights = std::unordered_map<int, float>();
	weights[0] = 1.0f;
	weights[7] = 1.0f;
	mActor->loopAnim( app::timeline(), weights );
}

void MultipleAnimationsDemo::fileDrop( FileDropEvent event )
{
	auto modelFile = loadFile( event.getFile( 0 ) );
	mActor = SkeletalMesh::create( AssimpLoader( modelFile ) );
}

void MultipleAnimationsDemo::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_f ) {
		app::setFullScreen(! isFullScreen() );
	} else if( event.getCode() == KeyEvent::KEY_ESCAPE ) {
		quit();
	}
}

void MultipleAnimationsDemo::mouseMove( MouseEvent event )
{
	mMouseHorizontalPos = float( event.getX() );
}

void MultipleAnimationsDemo::mouseDown( MouseEvent event )
{
	mCamUi.mouseDown( event.getPos() );
}

void MultipleAnimationsDemo::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mCamUi.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void MultipleAnimationsDemo::mouseWheel( MouseEvent event )
{
	//	mCamUi.mouseWheel( event.getWheelIncrement() );
}

void MultipleAnimationsDemo::resize()
{
	mCamUi.setWindowSize( getWindowSize() );
	mCamera.setAspectRatio( getWindowAspectRatio() );
//	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
}

void MultipleAnimationsDemo::update()
{	
	mFps = getAverageFps();
	model::Renderer::getLight()->position.x = mRotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
	model::Renderer::getLight()->position.z = mRotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
}

void MultipleAnimationsDemo::draw()
{
	// clear out the window with black
	gl::clear( Color(0.45f, 0.5f, 0.55f) );
	gl::setMatrices( mCamera );
	gl::scale( mScaleFactor * vec3( 2.5f ) );
	
	if ( mEnableWireframe )
		gl::enableWireframe();
	if( mDrawMesh ) {
		model::Renderer::draw( mActor );
	}
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	if( mDrawSkeleton) {
		model::Renderer::draw( mActor->getSkeleton() );
	}
	
	if( mDrawLabels ) {
		model::Renderer::drawLabels( mActor->getSkeleton(), mCamera );
	}
	
	mParams.draw();
	
}

CINDER_APP( MultipleAnimationsDemo, RendererGl )
