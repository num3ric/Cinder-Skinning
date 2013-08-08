#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/gl/Light.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"

#include "Resources.h"
#include "Node.h"
#include "Skeleton.h"
#include "SkinnedVboMesh.h"
#include "ModelSourceAssimp.h"
#include "SkinningRenderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace model;

const int NB_CLOUD_PARTICLES = 20;
const float SCENE_SIZE = 60.0f;

class ProceduralAnimApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void keyDown( KeyEvent event );
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
	gl::TextureRef dustParticleTex;
	gl::GlslProgRef mDustShader;
	
	float mFlapAngle, mFlapIncrement;
	float mShoulderAngle;
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
	mShoulderAngle = 0.0f;
	mParams.addParam( "Flap speed", &mFlapIncrement, "min=1.0 max=45.0 precision=1" );
	mParams.addParam( "Amplitude", &mAmplitude, "min=0.0 max=3.0 step=0.05 precision=2" );
	mParams.addParam( "Shoulder angle", &mShoulderAngle, "min=-1.57 max=0.5 step=0.05 precision=2" );
	
	for( int i=0; i<NB_CLOUD_PARTICLES; ++i ) {
		mDust.push_back( SCENE_SIZE * Vec3f( Rand::randFloat() - 0.5f,
									  Rand::randFloat() - 0.5f,
									  Rand::randFloat() - 0.5f ) );
	}
	dustParticleTex = gl::Texture::create( loadImage( getAssetPath( "particle.png" ) ) );
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
	mMayaCam.mouseDown( event.getPos() );
}

void ProceduralAnimApp::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void ProceduralAnimApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_f ) {
		app::setFullScreen( !isFullScreen() );
	}
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
	float h = mAmplitude * 0.5f * math<float>::sin( mFlapAngle );
	float t = mAmplitude * 0.5f * math<float>::sin( mFlapAngle - M_PI / 2 );

	SkeletonRef skeleton = mSkinnedVboBird->getSkeleton();		
	NodeRef midL = skeleton->getBone("Gannet_Lwing_mid");
	NodeRef midR = skeleton->getBone("Gannet_Rwing_mid");
	NodeRef tipL = skeleton->getBone("Gannet_Lwing_tip");
	NodeRef tipR = skeleton->getBone("Gannet_Rwing_tip");
	midL->setRelativeRotation( midL->getInitialRelativeRotation() * Quatf( Vec3f::xAxis(), t ) * Quatf( Vec3f::zAxis(), mShoulderAngle ) );
	midR->setRelativeRotation( midR->getInitialRelativeRotation() * Quatf( Vec3f::xAxis(), t ) * Quatf( Vec3f::zAxis(), mShoulderAngle ) );
	tipL->setRelativeRotation( Quatf( Vec3f::yAxis(), h ) );
	tipR->setRelativeRotation( Quatf( Vec3f::yAxis(), h ) );
	
	NodeRef head = skeleton->getBone("Gannet_head");
	head->setRelativeRotation( head->getInitialRelativeRotation().slerp(0.5f, mMayaCam.getCamera().getOrientation() )  );
	
	skeleton->getBone("Gannet_body")->setRelativePosition( Vec3f(0, -t, 0) );
	
	/* The mesh isn't automatically updated when the skeleton it contained is modified, so
	 * an update call is necessary (for now). This may change since I don't find it very
	 * intuitive... */
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
	gl::enableDepthRead();
	
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
		SkinningRenderer::draw( mSkinnedVboBird );
	}
	if ( mEnableWireframe )
		gl::disableWireframe();
	
	gl::disable( GL_LIGHTING );
	gl::disable( GL_NORMALIZE );
	
	if( mDrawSkeleton) {
		SkinningRenderer::draw( mSkinnedVboBird->getSkeleton() );
	}
	
	if( mDrawLabels ) {
		SkinningRenderer::drawLabels( mSkinnedVboBird->getSkeleton(), mMayaCam.getCamera() );
	}

	gl::popMatrices();
	
	Vec3f mRight, mUp;
	mMayaCam.getCamera().getBillboardVectors(&mRight, &mUp);
//	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::enableAlphaBlending();
	dustParticleTex->enableAndBind();
	
	mDustShader->bind();
	mDustShader->uniform( "tex", 0 );
	for( const auto& d : mDust ) {
		mDustShader->uniform( "transparency", 1.0f - math<float>::abs(d.z)/(SCENE_SIZE/2.0f) );
		gl::drawBillboard(d, Vec2f(60.f, 60.f), 0.0f, mRight, mUp);
	}
	mDustShader->unbind();
	
	dustParticleTex->unbind();

	mParams.draw();
}

CINDER_APP_NATIVE( ProceduralAnimApp, RendererGl )
