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

const int ROW_LEN = 10;
const int NUM_MONSTERS = ROW_LEN * ROW_LEN;
const float SPACING = 85.0f;

using namespace model;

class ArmyDemoApp : public AppNative {
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
	SkinnedVboMeshRef				mSkinnedVboMesh;
	
	MayaCamUI						mMayaCam;
	float							mMouseHorizontalPos;
	float							rotationRadius;
	Vec3f							mLightPos;
	
	int								mMeshIndex;
	float							mTime, mFps;
	params::InterfaceGl				mParams;
	bool mDrawSkeleton, mDrawMesh, mDrawAbsolute, mEnableWireframe;
};

void ArmyDemoApp::setup()
{
	model::Skeleton::mRenderMode = model::Skeleton::RenderMode::CLEANED;

	rotationRadius = 20.0f;
	mLightPos = Vec3f(10.0f, 20.0f, 20.0f);
	mMouseHorizontalPos = 0;
	mMeshIndex = 0;
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 250 ) );
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
	
	mSkinnedVboMesh = SkinnedVboMesh::create( loadModel( getAssetPath( "maggot3.md5mesh" ) ) );
}

void ArmyDemoApp::fileDrop( FileDropEvent event )
{
	try {
		fs::path modelFile = event.getFile( 0 );
		mSkinnedVboMesh = SkinnedVboMesh::create( loadModel( modelFile ) );
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
	mMayaCam.mouseDown( event.getPos() );
}

void ArmyDemoApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
}

void ArmyDemoApp::resize()
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
	gl::enableDepthWrite();
	gl::enableDepthRead();
}

void ArmyDemoApp::update()
{
	mFps = getAverageFps();
	mTime = 0.05f * mSkinnedVboMesh->getSkeleton()->getAnimDuration() * mMouseHorizontalPos / getWindowWidth();
}

void ArmyDemoApp::draw()
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

	gl::enable( GL_LIGHTING );
	gl::enable( GL_NORMALIZE );

	gl::scale(0.1f, 0.1f, 0.1f);
	
	if ( mEnableWireframe )
		gl::enableWireframe();
	
	for(int i=0; i < ROW_LEN; ++i) {
		for(int j=0; j < ROW_LEN; ++j ) {
			gl::pushModelView();
			gl::translate(SPACING * (i - 0.5f * ROW_LEN), 0, SPACING * (j - 0.5f * ROW_LEN));
			mSkinnedVboMesh->getSkeleton()->setPose( mTime + 2.0f*( i * j )/NUM_MONSTERS );
			if( mDrawMesh ) {
				mSkinnedVboMesh->update();
				SkinningRenderer::draw( mSkinnedVboMesh );
			}
			if( mDrawSkeleton) {
				SkinningRenderer::draw( mSkinnedVboMesh->getSkeleton(), mDrawAbsolute );
			}
			gl::popModelView();
		}
	}
	
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	mParams.draw();
	
}

CINDER_APP_NATIVE( ArmyDemoApp, RendererGl )
