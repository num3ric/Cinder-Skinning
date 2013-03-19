#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"

//#include "ModelIo.h"
#include "ModelSourceAssimp.h" //FIXME: including ModelIo.h only breaks the build
#include "SkinnedMesh.h"
#include "SkinnedVboMesh.h"

class SeymourDemo : public AppNative {
public:
	void setup();
	
	void resize();
	void keyDown( KeyEvent event );
	void mouseMove( MouseEvent event );
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
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
	
	int								mMeshIndex, mFps;
	params::InterfaceGl				mParams;
	bool mUseVbo, mDrawSkeleton, mDrawMesh, mDrawRelative, mEnableSkinning, mEnableWireframe;
};

void SeymourDemo::setup()
{
	rotationRadius = 20.0f;
	mLightPos = Vec3f(0, 20.0f, 0);
	mMouseHorizontalPos = 0;
	mMeshIndex = 0;
	mUseVbo = true;
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 250 ) );
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addSeparator();
	mParams.addParam( "Use VboMesh", &mUseVbo );
	mDrawMesh = true;
	mParams.addParam( "Draw Mesh", &mDrawMesh );
	mDrawSkeleton = false;
	mParams.addParam( "Draw Skeleton", &mDrawSkeleton );
	mDrawRelative = false;
	mParams.addParam( "Relative/Abolute skeleton", &mDrawRelative );
	mEnableSkinning = true;
	mParams.addParam( "Skinning", &mEnableSkinning );
	mEnableWireframe = false;
	mParams.addParam( "Wireframe", &mEnableWireframe );
	
	//	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
	
	mSkinnedMesh = make_shared<SkinnedMesh>( loadModel( getAssetPath( "astroboy_walk.dae" ) ) );
	app::console() << *mSkinnedMesh->getSkeleton();
	mSkinnedVboMesh = make_shared<SkinnedVboMesh>( loadModel( getAssetPath( "astroboy_walk.dae" ) ), mSkinnedMesh->getSkeleton() );
}

void SeymourDemo::fileDrop( FileDropEvent event )
{
	try {
		fs::path modelFile = event.getFile( 0 );
		mSkinnedMesh = make_shared<SkinnedMesh>( loadModel( modelFile, getFolderPath() ) );
		mSkinnedVboMesh = make_shared<SkinnedVboMesh>( loadModel( modelFile ), mSkinnedMesh->getSkeleton() );
		console() << *mSkinnedMesh->getSkeleton();
	}
	catch( ... ) {
		console() << "unable to load the asset!" << std::endl;
	};
}

void SeymourDemo::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_m ) {
		mDrawRelative = !mDrawRelative;
	} else if( event.getCode() == KeyEvent::KEY_s ) {
		mEnableSkinning = !mEnableSkinning;
	} else if( event.getCode() == KeyEvent::KEY_UP ) {
		mMeshIndex++;
	} else if( event.getCode() == KeyEvent::KEY_DOWN ) {
		mMeshIndex = math<int>::max(mMeshIndex - 1, 0);
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
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void SeymourDemo::resize()
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

void SeymourDemo::update()
{
	if( mUseVbo ) {
		float time = mSkinnedVboMesh->getSkeleton()->mAnimationDuration * mMouseHorizontalPos / getWindowWidth();
		mSkinnedVboMesh->update( time, mEnableSkinning );
	} else {
		float time = mSkinnedMesh->getSkeleton()->mAnimationDuration * mMouseHorizontalPos / getWindowWidth();
		mSkinnedMesh->update( time, mEnableSkinning );
	}
	mFps = getAverageFps();
	mLightPos.x = rotationRadius * math<float>::sin( app::getElapsedSeconds() );
	mLightPos.z = rotationRadius * math<float>::cos( app::getElapsedSeconds() );
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
	
	
	gl::drawVector(mLightPos, Vec3f::zero() );
	
	gl::enable( GL_LIGHTING );
	gl::enable( GL_NORMALIZE );
	
	if ( mEnableWireframe )
		gl::enableWireframe();
	if( mDrawMesh ) {
		if( mUseVbo ) {
			mSkinnedVboMesh->draw();
		} else {
			mSkinnedMesh->draw();
		}
	}
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	if( mDrawSkeleton) {
		if( mUseVbo ) {
			mSkinnedVboMesh->getSkeleton()->draw(mDrawRelative);
		} else {
			mSkinnedMesh->getSkeleton()->draw(mDrawRelative);
		}
	}
	
	params::InterfaceGl::draw();
	
}

CINDER_APP_NATIVE( SeymourDemo, RendererGl )
