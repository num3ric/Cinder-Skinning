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

#include "Treadmill.h"

using namespace model;

class SeymourDemo : public AppNative {
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
	SkinnedMeshRef					mSkinnedMesh;
	SkinnedVboMeshRef				mSkinnedVboMesh;
	
	MayaCamUI						mMayaCam;
	float							mMouseHorizontalPos;
	float							rotationRadius;
	Vec3f							mLightPos;
	
	float							mFps;
	params::InterfaceGl				mParams;
	bool mUseVbo, mDrawSkeleton, mDrawLabels, mDrawMesh, mDrawAbsolute, mEnableWireframe;
	bool mDrawTreadmill;
	
	std::unique_ptr<Treadmill> mTreadmill;
};

void SeymourDemo::setup()
{
	model::Skeleton::mRenderMode = model::Skeleton::RenderMode::CLEANED;
	
	mDrawTreadmill = true;
	mTreadmill = std::unique_ptr<Treadmill>( new Treadmill() );

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
	mDrawAbsolute = true;
	mParams.addParam( "Abolute skeleton", &mDrawAbsolute );
	mEnableWireframe = false;
	mParams.addParam( "Wireframe", &mEnableWireframe );
	
	//	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
	
	mSkinnedMesh = SkinnedMesh::create( loadModel( getAssetPath( "astroboy_walk.dae" ) ) );
//	app::console() << *mSkinnedMesh->getSkeleton();
	mSkinnedVboMesh = SkinnedVboMesh::create( loadModel( getAssetPath( "astroboy_walk.dae" ) ), mSkinnedMesh->getSkeleton(), nullptr );
	
//	mSkinnedMesh->getSkeleton()->loopAnim();
}

void SeymourDemo::fileDrop( FileDropEvent event )
{
//	try {
		fs::path modelFile = event.getFile( 0 );
		mSkinnedMesh = SkinnedMesh::create( loadModel( modelFile ) );
		mSkinnedVboMesh = SkinnedVboMesh::create( loadModel( modelFile ), mSkinnedMesh->getSkeleton() );
//	}
//	catch( ... ) {
//		console() << "unable to load the asset!" << std::endl;
//	};
	mDrawTreadmill = false;
}

void SeymourDemo::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_f ) {
		app::setFullScreen( !isFullScreen() );
	}
}

void SeymourDemo::mouseMove( MouseEvent event )
{
	mMouseHorizontalPos = float( event.getX() );	
}

void SeymourDemo::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void SeymourDemo::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void SeymourDemo::mouseWheel( MouseEvent event )
{
//	mMayaCam.mouseWheel( event.getWheelIncrement() );
}

void SeymourDemo::resize()
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
	gl::enableDepthRead();
}

void SeymourDemo::update()
{
	if( mUseVbo && mSkinnedVboMesh->hasSkeleton() ) {
		float time = mSkinnedVboMesh->getSkeleton()->getAnimDuration() * mMouseHorizontalPos / getWindowWidth();
		mSkinnedVboMesh->getSkeleton()->setPose( time );
		mSkinnedVboMesh->update();
	} else if( mSkinnedMesh->hasSkeleton() ) {
		float time = mSkinnedMesh->getSkeleton()->getAnimDuration() * mMouseHorizontalPos / getWindowWidth();
		mSkinnedMesh->getSkeleton()->setPose( time );
		mSkinnedMesh->update();
	}
	mFps = getAverageFps();
	mLightPos.x = rotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
	mLightPos.z = rotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
}

void SeymourDemo::draw()
{
	// clear out the window with black
	gl::clear( Color(0.45f, 0.5f, 0.55f) );
	gl::setMatrices( mMayaCam.getCamera() );
	gl::translate(0, -5.0f, 0.0f);
	
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
			SkinningRenderer::draw( mSkinnedVboMesh );
		} else {
			SkinningRenderer::draw( mSkinnedMesh );
		}
	}
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	if( mSkinnedVboMesh->getSkeleton() ) {
		if( mDrawSkeleton) {
			SkinningRenderer::draw( mSkinnedVboMesh->getSkeleton(), mDrawAbsolute );
		}
		
		if( mDrawLabels ) {
			SkinningRenderer::drawLabels( mSkinnedVboMesh->getSkeleton(),  mMayaCam.getCamera() );
		}
	}
	
	if( mDrawTreadmill ) {
		gl::disable( GL_CULL_FACE );
		mTreadmill->draw( mMouseHorizontalPos / getWindowWidth() );
	}
	
	mParams.draw();
	
}

CINDER_APP_NATIVE( SeymourDemo, RendererGl )
