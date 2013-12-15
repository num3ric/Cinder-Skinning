/* README: This Ogre-xml model has 15 different animation channels.
   These will serve as testcases for the development of the block's animation classes/API.
   And yes, the first animation is glitchy at the moment...
 */

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"

#include "ModelSourceAssimp.h"
#include "SkinnedMesh.h"
#include "SkinnedVboMesh.h"
#include "Skeleton.h"
#include "SkinningRenderer.h"

using namespace model;

class MultipleAnimationsDemo : public AppNative {
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
	
	SkinnedMeshRef					mSkinnedMesh;
	SkinnedVboMeshRef				mSkinnedVboMesh;
	
	MayaCamUI						mMayaCam;
	float							mMouseHorizontalPos;
	float							rotationRadius;
	Vec3f							mLightPos;
	
	float							mFps;
	params::InterfaceGl				mParams;
	bool mUseVbo, mDrawSkeleton, mDrawLabels, mDrawMesh, mDrawRelative, mEnableWireframe;
	int mAnimId;
};

void MultipleAnimationsDemo::playAnim()
{
	mSkinnedMesh->getSkeleton()->playAnim( app::timeline(), mAnimId );
}

void MultipleAnimationsDemo::loopAnim()
{
	mSkinnedMesh->getSkeleton()->loopAnim( app::timeline(), mAnimId );
}

void MultipleAnimationsDemo::stopAnim()
{
	mSkinnedMesh->getSkeleton()->stop();
}

void MultipleAnimationsDemo::setup()
{
	model::Skeleton::mRenderMode = model::Skeleton::RenderMode::CLEANED;
	
	rotationRadius = 20.0f;
	mLightPos = Vec3f(0, 20.0f, 0);
	mMouseHorizontalPos = 0;
	mUseVbo = true;
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 250 ) );
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addSeparator();
	mParams.addParam( "Use VboMesh", &mUseVbo );
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
	mParams.addParam( "Cycle Id", &mAnimId, "min=0 max=14" );
	mParams.addButton( "Play Anim", std::bind( &MultipleAnimationsDemo::playAnim, this) );
	mParams.addButton( "Loop Anim", std::bind( &MultipleAnimationsDemo::loopAnim, this) );
	mParams.addButton( "Stop Anim", std::bind( &MultipleAnimationsDemo::stopAnim, this) );
	
	mSkinnedMesh = SkinnedMesh::create( loadModel( getAssetPath( "Sinbad.mesh.xml" ) ) );
	app::console() << mSkinnedMesh->getSkeleton().get();
	mSkinnedVboMesh = SkinnedVboMesh::create( loadModel( getAssetPath( "Sinbad.mesh.xml" ) ), mSkinnedMesh->getSkeleton() );
	
	
	// Blend both animations with a factor of one since they affect independent bones (lower vs upper body).
	// mSkinnedMesh->getSkeleton()->loopAnim( { {0, 1.0f}, {7, 1.0f} } ); // this beautiful C++11 syntax is not supported on older version of vc11
	auto weights = std::unordered_map<int, float>();
	weights[0] = 1.0f;
	weights[7] = 1.0f;
	mSkinnedMesh->getSkeleton()->loopAnim( app::timeline(), weights );
}

void MultipleAnimationsDemo::fileDrop( FileDropEvent event )
{
	fs::path modelFile = event.getFile( 0 );
	mSkinnedMesh = SkinnedMesh::create( loadModel( modelFile ) );
	mSkinnedVboMesh = SkinnedVboMesh::create( loadModel( modelFile ), mSkinnedMesh->getSkeleton() );
}

void MultipleAnimationsDemo::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_f ) {
		app::setFullScreen(! isFullScreen() );
	}
}

void MultipleAnimationsDemo::mouseMove( MouseEvent event )
{
	mMouseHorizontalPos = float( event.getX() );
}

void MultipleAnimationsDemo::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void MultipleAnimationsDemo::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void MultipleAnimationsDemo::mouseWheel( MouseEvent event )
{
	//	mMayaCam.mouseWheel( event.getWheelIncrement() );
}

void MultipleAnimationsDemo::resize()
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
//	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
}

void MultipleAnimationsDemo::update()
{	
	mFps = getAverageFps();
	mLightPos.x = rotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
	mLightPos.z = rotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
}

void MultipleAnimationsDemo::draw()
{
	// clear out the window with black
	gl::clear( Color(0.45f, 0.5f, 0.55f) );
	gl::setMatrices( mMayaCam.getCamera() );
	gl::scale(2.5f, 2.5f, 2.5f);
	
	
	gl::Light light( gl::Light::DIRECTIONAL, 0 );
	light.setAmbient( Color::white() );
	light.setDiffuse( Color::white() );
	light.setSpecular( Color::white() );
	light.lookAt( mLightPos, Vec3f::zero() );
	light.update( mMayaCam.getCamera() );
	light.enable();
//	gl::drawVector(mLightPos, Vec3f::zero() );
	
	gl::enable( GL_LIGHTING );
	gl::enable( GL_NORMALIZE );
	
	if ( mEnableWireframe )
		gl::enableWireframe();
	if( mDrawMesh ) {
		if( mUseVbo ) {
			mSkinnedVboMesh->update();
			SkinningRenderer::draw( mSkinnedVboMesh );
		} else {
			mSkinnedMesh->update();
			SkinningRenderer::draw( mSkinnedMesh );
		}
	}
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	if( mDrawSkeleton) {
		SkinningRenderer::draw( mSkinnedVboMesh->getSkeleton() );
	}
	
	if( mDrawLabels ) {
		SkinningRenderer::drawLabels( mSkinnedVboMesh->getSkeleton(), mMayaCam.getCamera() );
	}
	
	mParams.draw();
	
}

CINDER_APP_NATIVE( MultipleAnimationsDemo, RendererGl )
