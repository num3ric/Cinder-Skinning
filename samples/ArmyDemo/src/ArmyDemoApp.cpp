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
#include "model/Skeleton.h"
#include "model/Renderer.h"

const int ROW_LEN = 10;
const int NUM_MONSTERS = ROW_LEN * ROW_LEN;
const float SPACING = 85.0f;

class ArmyDemoApp : public App {
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
	model::SkeletalMeshRef			mSkeletalMesh;
	
	CameraPersp						mCamera;
	CameraUi						mCamUi;
	float							mMouseHorizontalPos;
	vec3							mLightPos;
	
	int								mMeshIndex;
	float							mTime, mFps;
	params::InterfaceGl				mParams;
	bool mDrawSkeleton, mDrawMesh, mDrawAbsolute, mEnableWireframe;
};

void ArmyDemoApp::setup()
{
	model::Skeleton::sRenderMode = model::Skeleton::RenderMode::CLEANED;

	mLightPos = vec3(10.0f, 20.0f, 20.0f);
	mMouseHorizontalPos = 0;
	mMeshIndex = 0;
	mParams = params::InterfaceGl( "Parameters", ivec2( 200, 250 ) );
	mParams.addParam( "Fps", &mFps, "", true );
	mParams.addSeparator();
	mDrawMesh = true;
	mParams.addParam( "Draw Mesh", &mDrawMesh );
	mDrawSkeleton = false;
	mParams.addParam( "Draw Skeleton", &mDrawSkeleton );
	mDrawAbsolute = true;
	mParams.addParam( "Relative/Abolute skeleton", &mDrawAbsolute );
	mEnableWireframe = false;
	mParams.addParam( "Wireframe", &mEnableWireframe );
	
	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
	
	mSkeletalMesh = model::SkeletalMesh::create( model::AssimpLoader( loadAsset( "maggot3.md5mesh" ) ) );
}

void ArmyDemoApp::fileDrop( FileDropEvent event )
{
	try {
		mSkeletalMesh = model::SkeletalMesh::create( model::AssimpLoader( loadFile( event.getFile(0) ) ) );
	}
	catch( ... ) {
		console() << "unable to load the asset!" << std::endl;
	};
}

void ArmyDemoApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_m ) {
		mDrawAbsolute = !mDrawAbsolute;
	} else if( event.getCode() == KeyEvent::KEY_UP ) {
		mMeshIndex++;
	} else if( event.getCode() == KeyEvent::KEY_DOWN ) {
		mMeshIndex = math<int>::max(mMeshIndex - 1, 0);
	} else if( event.getCode() == KeyEvent::KEY_f ) {
		app::setFullScreen( !isFullScreen() );
	}
}

void ArmyDemoApp::mouseMove( MouseEvent event )
{
	mMouseHorizontalPos = float( event.getX() );
}

void ArmyDemoApp::mouseDown( MouseEvent event )
{
	mCamUi.mouseDown( event.getPos() );
}

void ArmyDemoApp::mouseDrag( MouseEvent event )
{
	mCamUi.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void ArmyDemoApp::resize()
{
	mCamUi.setWindowSize( getWindowSize() );
	mCamera.setAspectRatio( getWindowAspectRatio() );
	gl::enableDepthWrite();
	gl::enableDepthRead();
}

void ArmyDemoApp::update()
{
	mFps = getAverageFps();
	mTime = 0.05f * mSkeletalMesh->getAnimDuration() * mMouseHorizontalPos / getWindowWidth();
}

void ArmyDemoApp::draw()
{
	gl::clear( Color::black() );
	gl::setMatrices( mCamera );
	gl::translate( 0, -5.0f, 0.0f );
	gl::scale( vec3( 0.1f ) );
	
	if ( mEnableWireframe )
		gl::enableWireframe();
	
	for(int i=0; i < ROW_LEN; ++i) {
		for(int j=0; j < ROW_LEN; ++j ) {
			gl::pushModelMatrix();
			gl::translate(SPACING * (i - 0.5f * ROW_LEN), 0, SPACING * (j - 0.5f * ROW_LEN));
			mSkeletalMesh->setPose( mTime + 2.0f*( i * j )/NUM_MONSTERS );
			if( mDrawMesh ) {
				model::Renderer::draw( mSkeletalMesh );
			}
			if( mDrawSkeleton) {
				model::Renderer::draw( mSkeletalMesh->getSkeleton() );
			}
			gl::popModelMatrix();
		}
	}
	
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	mParams.draw();
	
}

CINDER_APP( ArmyDemoApp, RendererGl )
