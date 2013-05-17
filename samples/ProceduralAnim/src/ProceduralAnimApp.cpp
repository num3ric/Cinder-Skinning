#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"

#include "Node.h"
#include "Skeleton.h"
#include "SkinnedVboMesh.h"
#include "ModelSourceAssimp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace model;

class ProceduralAnimApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void resize();
	void update();
	void draw();
private:
	
	MayaCamUI			mMayaCam;
	float				mMouseHorizontalPos;
	float				mRotationRadius;
	Vec3f				mLightPos;
	
	SkinnedVboMeshRef	mSkinnedVboBird;
	
	params::InterfaceGl	mParams;
	
	bool mDrawSkeleton, mDrawLabels, mDrawMesh, mEnableWireframe;
	float mFrequency, mAmplitude;

};

void ProceduralAnimApp::setup()
{
	mSkinnedVboBird = SkinnedVboMesh::create( loadModel( getAssetPath( "gannet rig2.DAE" ) ) );
	
	mRotationRadius = 20.0f;
	mLightPos = Vec3f(0, 20.0f, 0);
	mMouseHorizontalPos = 0;
	
	
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 250 ) );
	mDrawMesh = true;
	mParams.addParam( "Draw Mesh", &mDrawMesh );
	mDrawSkeleton = false;
	mParams.addParam( "Draw Skeleton", &mDrawSkeleton );
	mDrawLabels = false;
	mParams.addParam( "Draw Labels", &mDrawLabels );
	mEnableWireframe = false;
	mParams.addParam( "Wireframe", &mEnableWireframe );
	mParams.addSeparator();
	mFrequency = 5.0f;
	mAmplitude = 1.0f;
	mParams.addParam( "Frequency", &mFrequency, "min=0.1 max=8.0 step=0.05 precision=2" );
	mParams.addParam( "Amplitude", &mAmplitude, "min=0.1 max=3.0 step=0.05 precision=2" );
	
	gl::enableDepthRead();
}

void ProceduralAnimApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void ProceduralAnimApp::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void ProceduralAnimApp::resize()
{
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
}

void ProceduralAnimApp::update()
{
	mLightPos.x = mRotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
	mLightPos.z = mRotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
	
	float e = (float)getElapsedSeconds();
	if ( mSkinnedVboBird ) {
		SkeletonRef skeleton = mSkinnedVboBird->getSkeleton();
		std::map<string, NodeRef> bones = skeleton->getBoneNames();
		
		NodeRef midL = skeleton->getBone("Gannet_Lwing_mid");
		NodeRef midR = skeleton->getBone("Gannet_Rwing_mid");
		NodeRef tipL = skeleton->getBone("Gannet_Lwing_tip");
		NodeRef tipR = skeleton->getBone("Gannet_Rwing_tip");
		
		float h = mAmplitude * 0.5f * math<float>::sin( mFrequency * e );
		float m = mAmplitude * 0.5f * math<float>::sin( mFrequency * (e + M_PI) );
		Vec3f axis(1, 0, 0);
		midL->setRelativeRotation( midL->getInitialRelativeRotation() * Quatf( axis, m ) );
		midR->setRelativeRotation( midR->getInitialRelativeRotation() * Quatf( axis, m ) );
		tipL->setRelativeRotation( tipL->getInitialRelativeRotation() * Quatf( axis, h ) );
		tipR->setRelativeRotation( tipR->getInitialRelativeRotation() * Quatf( axis, h ) );
		
		NodeRef head = skeleton->getBone("Gannet_head");
		head->setRelativeRotation( head->getInitialRelativeRotation().slerp(0.5f, mMayaCam.getCamera().getOrientation() )  );
	}
	
	mSkinnedVboBird->update();
}

void ProceduralAnimApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
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
		
	if ( mEnableWireframe )
		gl::enableWireframe();
	if( mDrawMesh ) {
		mSkinnedVboBird->draw();
	}
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	if( mDrawSkeleton) {
		mSkinnedVboBird->getSkeleton()->draw();
	}
	
	if( mDrawLabels ) {
		mSkinnedVboBird->getSkeleton()->drawLabels( mMayaCam.getCamera() );
	}
	
	mParams.draw();
}

CINDER_APP_NATIVE( ProceduralAnimApp, RendererGl )
