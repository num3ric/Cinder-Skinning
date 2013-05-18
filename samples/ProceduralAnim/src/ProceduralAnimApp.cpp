#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"

#include "Node.h"
#include "Skeleton.h"
#include "SkinnedVboMesh.h"
#include "ModelSourceAssimp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace model;

const float SCENE_SIZE = 70.0f;

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
	float mAmplitude;
	
	vector<Vec3f> mDust;
	
	float mFlapAngle, mFlapIncrement;
};

void ProceduralAnimApp::setup()
{
	mSkinnedVboBird = SkinnedVboMesh::create( loadModel( getAssetPath( "gannet rig2.DAE" ) ) );
	
	mRotationRadius = 20.0f;
	mLightPos = Vec3f(0, 20.0f, 0);
	mMouseHorizontalPos = 0;
	
	console() << *mSkinnedVboBird->getSkeleton();
	
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
	mFlapAngle = 0.0f;
	mFlapIncrement = 18.5f;
	mAmplitude = 1.0f;
	mParams.addParam( "Flap speed", &mFlapIncrement, "min=1.0 max=45.0 precision=1" );
	mParams.addParam( "Amplitude", &mAmplitude, "min=0.0 max=3.0 step=0.05 precision=2" );
	
	for( int i=0; i<100; ++i ) {
		mDust.push_back( SCENE_SIZE * Vec3f( Rand::randFloat() - 0.5f,
									  Rand::randFloat() - 0.5f,
									  Rand::randFloat() - 0.5f ) );
	}
	
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
	
	mFlapAngle += 0.01f * mFlapIncrement;
	
	if ( mSkinnedVboBird ) {
		SkeletonRef skeleton = mSkinnedVboBird->getSkeleton();
		std::map<string, NodeRef> bones = skeleton->getBoneNames();
		
		NodeRef midL = skeleton->getBone("Gannet_Lwing_mid");
		NodeRef midR = skeleton->getBone("Gannet_Rwing_mid");
		NodeRef tipL = skeleton->getBone("Gannet_Lwing_tip");
		NodeRef tipR = skeleton->getBone("Gannet_Rwing_tip");
		
		float h = mAmplitude * 0.5f * math<float>::sin( mFlapAngle );
		float t = mAmplitude * 0.5f * math<float>::sin( mFlapAngle - M_PI_2 );
		Vec3f axism(1, 0, 0), axist(0, 1, 0);
		midL->setRelativeRotation( midL->getInitialRelativeRotation() * Quatf( axism, t ) );
		midR->setRelativeRotation( midR->getInitialRelativeRotation() * Quatf( axism, t ) );
		tipL->setRelativeRotation( Quatf( axist, h ) );
		tipR->setRelativeRotation( Quatf( axist, h ) );
		
		NodeRef head = skeleton->getBone("Gannet_head");
		head->setRelativeRotation( head->getInitialRelativeRotation().slerp(0.5f, mMayaCam.getCamera().getOrientation() )  );
		
		skeleton->getBone("Gannet_body")->setRelativePosition( Vec3f(0, -t, 0) );
	}
	
	mSkinnedVboBird->update();
	
	for( auto& d : mDust ) {
		d.z -= 0.75f;
		d.z = ( d.z < -SCENE_SIZE/2 ) ? SCENE_SIZE/2 : d.z;
	}
}

void ProceduralAnimApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0.4f, 0.5f, 0.6f ) );
	
	gl::setMatrices( mMayaCam.getCamera() );
	
	gl::Light light( gl::Light::DIRECTIONAL, 0 );
	light.setAmbient( Color::white() );
	light.setDiffuse( Color::white() );
	light.setSpecular( Color::white() );
	light.lookAt( mLightPos, Vec3f::zero() );
	light.update( mMayaCam.getCamera() );
	light.enable();
	
	gl::pushMatrices();
	float e = (float)getElapsedSeconds();
	gl::translate( math<float>::sin( 1.3f * e ), math<float>::cos( 2.7f * e ) );
	gl::enable( GL_LIGHTING );
	gl::enable( GL_NORMALIZE );
		
	if ( mEnableWireframe )
		gl::enableWireframe();
	if( mDrawMesh ) {
		mSkinnedVboBird->draw();
	}
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	gl::disable( GL_LIGHTING );
	gl::disable( GL_NORMALIZE );
	
	if( mDrawSkeleton) {
		mSkinnedVboBird->getSkeleton()->draw();
	}
	
	if( mDrawLabels ) {
		mSkinnedVboBird->getSkeleton()->drawLabels( mMayaCam.getCamera() );
	}

	gl::popMatrices();
	
	Vec3f mRight, mUp;
	mMayaCam.getCamera().getBillboardVectors(&mRight, &mUp);
	for( auto d : mDust ) {
		gl::drawBillboard(d, Vec2f(0.2f, 0.2f), 0.0f, mRight, mUp);
	}
	
	mParams.draw();
}

CINDER_APP_NATIVE( ProceduralAnimApp, RendererGl )
