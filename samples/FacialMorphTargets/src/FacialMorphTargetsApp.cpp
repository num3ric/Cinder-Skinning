#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"

#include "model/Renderer.h"
#include "model/AssimpLoader.h"
#include "model/MorphedMesh.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class FacialMorphTargetsApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void resize() override;
	void update() override;
	void draw() override;

	model::MorphedMeshRef	mJane;
	CameraPersp				mCamera;
	CameraUi				mCamUi;
	params::InterfaceGlRef	mParams;
	
	float mAngerWeight, mOpenSmileWeight, mSurpriseWeight;
};

void FacialMorphTargetsApp::setup()
{
	using namespace model;
	// http://www.facegen.com/sampleExports.htm

	std::vector<ci::DataSourceRef> morphTargets;
	// Up to 10 morph targets supported. Order-dependent. Shader has to be manually updated.
	morphTargets.emplace_back( loadAsset( "jane/Jane_hi_all_ExpressionAnger.obj" ) );
	morphTargets.emplace_back( loadAsset( "jane/Jane_hi_all_ExpressionSmileOpen.obj" ) );
	morphTargets.emplace_back( loadAsset( "jane/Jane_hi_all_ExpressionSurprise.obj" ) );
	mJane = MorphedMesh::create( AssimpLoader( loadAsset( "jane/Jane_hi_all.obj" ), AssimpLoader::Settings().morphTargets( morphTargets ) ) );
	gl::enableDepthWrite();
	gl::enableDepthRead();
	
	
	mParams = params::InterfaceGl::create( "Parameters", ivec2( 200, 250 ) );
	mAngerWeight = mOpenSmileWeight = mSurpriseWeight = 0.0f;
	mParams->addParam( "Anger weight", &mAngerWeight ).min(0.0f).max(1.0f).step(0.025f);
	mParams->addParam( "Open smile weight", &mOpenSmileWeight ).min(0.0f).max(1.0f).step(0.025f);
	mParams->addParam( "Surprise weight", &mSurpriseWeight ).min(0.0f).max(1.0f).step(0.025f);
	
	model::Renderer::getLight()->position.y = 200.0f;

	mCamera.lookAt( vec3( 0.0f, 0.0f, 60.0f ), vec3( 0.0f ) );
	mCamUi = CameraUi( &mCamera );
}

void FacialMorphTargetsApp::mouseDown( MouseEvent event )
{
	mCamUi.mouseDown( event.getPos() );
}

void FacialMorphTargetsApp::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mCamUi.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void FacialMorphTargetsApp::resize()
{
	mCamUi.setWindowSize( getWindowSize() );
	mCamera.setAspectRatio( getWindowAspectRatio() );
}

void FacialMorphTargetsApp::update()
{
	model::Renderer::getLight()->position.x = 200.0f * sin( float( app::getElapsedSeconds() ) );
	model::Renderer::getLight()->position.z = 200.0f * cos( float( app::getElapsedSeconds() ) );
	
	//Could also be moved to updateFn in params
	mJane->setMorphTargetWeight( 0, mAngerWeight );
	mJane->setMorphTargetWeight( 1, mOpenSmileWeight );
	mJane->setMorphTargetWeight( 2, mSurpriseWeight );
}

void FacialMorphTargetsApp::draw()
{
	gl::clear( Color::gray(0.4f) );
	gl::setMatrices( mCamera );
	gl::scale( vec3( 0.1 ) );
	gl::translate( 0, -5.0f, 0.0f );

	model::Renderer::draw( mJane );

	mParams->draw();
}

CINDER_APP( FacialMorphTargetsApp, RendererGl )
