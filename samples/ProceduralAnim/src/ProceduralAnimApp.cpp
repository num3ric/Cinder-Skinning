#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"

#include "Resources.h"

#include "model/AssimpLoader.h"
#include "model/Node.h"
#include "model/Skeleton.h"
#include "model/SkeletalMesh.h"
#include "model/Renderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const int NB_CLOUD_PARTICLES = 20;
const float SCENE_SIZE = 60.0f;

class ProceduralAnimApp : public App {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
	void resize();
	void update();
	void draw();
private:

	CameraPersp			mCamera;
	CameraUi			mCamUi;
	float				mMouseHorizontalPos;
	float				mRotationRadius;
	
	model::SkeletalMeshRef	mBird;
	
	params::InterfaceGl	mParams;
	
	bool mDrawSkeleton, mDrawLabels, mDrawMesh, mEnableWireframe;
	float mAmplitude;
	float mScaleFactor;
	
	vector<vec3> mDust;
	gl::TextureRef dustParticleTex;
	gl::GlslProgRef mDustShader;
	
	float mFlapAngle, mFlapIncrement;
	float mShoulderAngle;
};

void ProceduralAnimApp::setup()
{
	mBird = model::SkeletalMesh::create( model::AssimpLoader( loadAsset( "gannet rig2.DAE" ) ) );
	mScaleFactor = 50.0f;
	mRotationRadius = 20.0f;
	mMouseHorizontalPos = 0;
	
//	console() << *mBird->getSkeleton();
	
	mParams = params::InterfaceGl( "Parameters", ivec2( 200, 250 ) );
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
	mShoulderAngle = 0.0f;
	mParams.addParam( "Flap speed", &mFlapIncrement, "min=1.0 max=45.0 precision=1" );
	mParams.addParam( "Amplitude", &mAmplitude, "min=0.0 max=3.0 step=0.05 precision=2" );
	mParams.addParam( "Shoulder angle", &mShoulderAngle, "min=-1.57 max=0.5 step=0.05 precision=2" );
	
	for( int i=0; i<NB_CLOUD_PARTICLES; ++i ) {
		mDust.push_back( SCENE_SIZE * vec3( Rand::randFloat() - 0.5f,
									  Rand::randFloat() - 0.5f,
									  Rand::randFloat() - 0.5f ) );
	}
	dustParticleTex = gl::Texture::create( loadImage( loadAsset( "particle.png" ) ) );
	try {
		mDustShader = gl::GlslProg::create( app::loadResource(RES_CLOUD_VERT), app::loadResource(RES_CLOUD_FRAG) );
	}
	catch( gl::GlslProgCompileExc &exc ) {
		app::console() << "Shader compile error: " << std::endl;
		app::console() << exc.what();
	}
}

void ProceduralAnimApp::mouseDown( MouseEvent event )
{
	mCamUi.mouseDown( event.getPos() );
}

void ProceduralAnimApp::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mCamUi.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void ProceduralAnimApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_f ) {
		app::setFullScreen( !isFullScreen() );
	}
}

void ProceduralAnimApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
	mCamUi.setWindowSize( getWindowSize() );
}

void ProceduralAnimApp::update()
{
	model::Renderer::getLight()->position.x = mRotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
	model::Renderer::getLight()->position.z = mRotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
	
	mFlapAngle += 0.01f * mFlapIncrement;
	float h = mAmplitude * 0.5f * math<float>::sin( mFlapAngle );
	float t = mAmplitude * 0.5f * math<float>::sin( mFlapAngle - float(M_PI / 2.0f) );
	
	using namespace model;
	
	SkeletonRef skeleton = mBird->getSkeleton();
	NodeRef midL = skeleton->getBone("Gannet_Lwing_mid");
	NodeRef midR = skeleton->getBone("Gannet_Rwing_mid");
	NodeRef tipL = skeleton->getBone("Gannet_Lwing_tip");
	NodeRef tipR = skeleton->getBone("Gannet_Rwing_tip");
	midL->setRelativeRotation(  angleAxis( mShoulderAngle, vec3(0,0,1) ) * angleAxis( t, vec3(1,0,0) ) * midL->getInitialRelativeRotation() );
	midR->setRelativeRotation( angleAxis( mShoulderAngle, vec3(0,0,1) ) * angleAxis( t, vec3(1,0,0) ) * midR->getInitialRelativeRotation() );
	tipL->setRelativeRotation( angleAxis( h, vec3(0,1,0) ) );
	tipR->setRelativeRotation( angleAxis( h, vec3(0,1,0) ) );
	
	NodeRef head = skeleton->getBone("Gannet_head");
	head->setRelativeRotation( slerp( head->getInitialRelativeRotation(), mCamera.getOrientation(), 0.5f )  );
	
	skeleton->getBone("Gannet_body")->setRelativePosition( vec3(0, -t, 0) );
	
	/* The mesh isn't automatically updated when the skeleton it contained is modified, so
	 * an update call is necessary (for now). This may change since I don't find it very
	 * intuitive... */
	mBird->update();
	
	for( auto& d : mDust ) {
		d.z -= 0.75f;
		d.z = ( d.z < -SCENE_SIZE/2 ) ? SCENE_SIZE/2 : d.z;
	}
}

void ProceduralAnimApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0.4f, 0.5f, 0.6f ) );
	gl::setMatrices( mCamera );
	gl::enableDepthRead();
    gl::enableDepthWrite();
	
	gl::pushMatrices();
	float e = (float)getElapsedSeconds();
	gl::translate( sin( 1.3f * e ), cos( 2.7f * e ) );
	gl::scale( vec3( mScaleFactor ) );
	
	if ( mEnableWireframe )
		gl::enableWireframe();
	if( mDrawMesh ) {
		model::Renderer::draw( mBird );
	}
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	if( mDrawSkeleton) {
		model::Renderer::draw( mBird->getSkeleton() );
	}
	
	if( mDrawLabels ) {
		model::Renderer::drawLabels( mBird->getSkeleton(), mCamera );
	}

	gl::popMatrices();
	
	vec3 mRight, mUp;
	mCamera.getBillboardVectors(&mRight, &mUp);
//	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::enableAlphaBlending();
	dustParticleTex->bind();
	
	mDustShader->bind();
	mDustShader->uniform( "tex", 0 );
	for( const auto& d : mDust ) {
		mDustShader->uniform( "transparency", 1.0f - abs(d.z)/(SCENE_SIZE/2.0f) );
		gl::drawBillboard(d, vec2(60.f, 60.f), 0.0f, mRight, mUp);
	}
	
	dustParticleTex->unbind();

	mParams.draw();
}

CINDER_APP( ProceduralAnimApp, RendererGl )
