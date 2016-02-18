#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"

#include "AssimpLoader.h"
#include "SkeletalMesh.h"
#include "Skeleton.h"
#include "Renderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AssimpApp : public AppNative {
public:
	void setup() override;
	void touchesMoved( TouchEvent event ) override;
	void update() override;
	void draw() override;
	
	model::SkeletalMeshRef		mSkeletalMesh;
	MayaCamUI					mMayaCam;
	float						mTouchHorizontalPos;
	float						mRotationRadius;
};

void AssimpApp::setup()
{
	using namespace model;
	mSkeletalMesh = SkeletalMesh::create( AssimpLoader( loadAsset( "astroboy_walk.dae" ) ) );
	mRotationRadius = 20.0f;
	mTouchHorizontalPos = 0;
	
	CameraPersp cam = mMayaCam.getCamera();
	cam.setAspectRatio( getWindowAspectRatio() );
	mMayaCam.setCurrentCam( cam );
	gl::enableDepthRead();
}

void AssimpApp::touchesMoved( TouchEvent event )
{
	auto touches = event.getTouches();
	
	mTouchHorizontalPos = float( toPixels( touches[0].getX() ) );
}

void AssimpApp::update()
{
	if( mSkeletalMesh->getSkeleton() ) {
		float time = mSkeletalMesh->getAnimDuration() * mTouchHorizontalPos / toPixels(getWindowWidth());
		mSkeletalMesh->setPose( time );
	}
	
	model::Renderer::getLight()->position.x = mRotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
	model::Renderer::getLight()->position.z = mRotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
}

void AssimpApp::draw()
{
	// clear out the window with black
	gl::clear( Color(0.45f, 0.5f, 0.55f) );
	gl::setMatrices( mMayaCam.getCamera() );
	gl::translate(0, -5.0f, 0.0f);
	
	model::Renderer::draw( mSkeletalMesh );
}

CINDER_APP_NATIVE( AssimpApp, RendererGl )
