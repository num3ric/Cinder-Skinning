#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "cinder/Log.h"

#include "model/AssimpLoader.h"
#include "model/SkeletalMesh.h"
#include "model/SkeletalTriMesh.h"
#include "model/Skeleton.h"
#include "model/Renderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SeymourDemo : public App {
public:
	void setup() override;
	void resize() override;
	void keyDown( KeyEvent event ) override;
	void mouseMove( MouseEvent event ) override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void fileDrop( FileDropEvent event ) override;
	void update() override;
	void draw() override;
private:
	model::SkeletalTriMeshRef		mSkeletalTriMesh;
	model::SkeletalMeshRef			mSkeletalMesh;

	CameraPersp						mCamera;
	CameraUi						mCamUi;
	float							mMouseHorizontalPos;
	float							mRotationRadius;
	
	float							mFps;
	float							mScaleFactor;
	float							mBgColor;
	params::InterfaceGl				mParams;
	bool mUseVbo, mDrawSkeleton, mDrawLabels, mDrawMesh, mEnableWireframe, mBoundingBox, mDrawAllSections;
	int mSectionId;
};

void SeymourDemo::setup()
{
	mRotationRadius = 20.0f;
	mMouseHorizontalPos = 0;
	mUseVbo = true;
	mParams = params::InterfaceGl( "Parameters", ivec2( 200, 250 ) );
	mParams.addParam( "Fps", &mFps, "", true );
	mBgColor = 0.45f;
	mParams.addParam( "Bg color", &mBgColor ).step( 0.05).min( 0.0 ).max( 1.0 );
	// Latest assimp build requires a scaling factor of 100.0f. Don't ask.
	mScaleFactor = 100.0f;
	mParams.addParam( "Scale factor", &mScaleFactor ).step( 0.1 );
	mParams.addSeparator();
	mParams.addParam( "Use VboMesh", &mUseVbo );
	mDrawMesh = true;
	mParams.addParam( "Draw Mesh", &mDrawMesh );
	mDrawAllSections = true;
	mParams.addParam( "Draw all mesh sections", &mDrawAllSections );
	mSectionId = 0;
	mParams.addParam( "Section index", &mSectionId ).min(0.0);
	mDrawSkeleton = false;
	mParams.addParam( "Draw Skeleton", &mDrawSkeleton );
	mDrawLabels = false;
	mParams.addParam( "Draw Labels", &mDrawLabels );
	mEnableWireframe = false;
	mParams.addParam( "Wireframe", &mEnableWireframe );
	mBoundingBox = false;
	mParams.addParam( "Bounding Box", &mBoundingBox );
		
	const auto& loader = model::AssimpLoader( loadAsset( "astroboy_walk.dae" ) );
	mSkeletalTriMesh = model::SkeletalTriMesh::create( loader );
	mSkeletalMesh = model::SkeletalMesh::create( loader, mSkeletalTriMesh->getSkeleton() );

	mCamera.lookAt( vec3( 30.0f, 15.0f, 40.0f ), vec3( 0.0f ) );
	mCamUi = CameraUi( &mCamera );

	gl::getStockShader( gl::ShaderDef().color() )->bind();
	gl::enableDepthWrite();
	gl::enableDepthRead();
	gl::enableAlphaBlending();
}

void SeymourDemo::fileDrop( FileDropEvent event )
{
	try {
		const auto& loader = model::AssimpLoader( loadFile( event.getFile( 0 ) ),  model::AssimpLoader::Settings().loadAnims(false) );
		mSkeletalTriMesh = model::SkeletalTriMesh::create( loader );
		mSkeletalMesh = model::SkeletalMesh::create( loader, mSkeletalTriMesh->getSkeleton() );		
	}
	catch( ... ) {
		CI_LOG_E( "Unable to load the asset." );
	};
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
	mCamUi.mouseDown( event.getPos() );
}

void SeymourDemo::mouseDrag( MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mCamUi.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void SeymourDemo::resize()
{
	mCamUi.setWindowSize( getWindowSize() );
	mCamera.setAspectRatio( getWindowAspectRatio() );
	gl::enableDepthRead();
}

void SeymourDemo::update()
{
	if( mSkeletalMesh->hasAnimations() ) {
		float time = mSkeletalMesh->getAnimDuration() * mMouseHorizontalPos / math<float>::max( 200.0f, getWindowWidth() );
		mSkeletalMesh->setPose( time );
		mSkeletalTriMesh->setPose( time );
	}

	mFps = getAverageFps();
	model::Renderer::getLight()->position.x = mRotationRadius * math<float>::sin( float( app::getElapsedSeconds() ) );
	model::Renderer::getLight()->position.z = mRotationRadius * math<float>::cos( float( app::getElapsedSeconds() ) );
}

void SeymourDemo::draw()
{
	gl::clear( Color( mBgColor * 0.9, mBgColor, mBgColor * 1.1 ) );
	gl::setMatrices( mCamera );
	gl::translate(0, -5.0f, 0.0f);
	gl::scale( vec3( mScaleFactor ) );

	if( mDrawMesh ) {
		if ( mEnableWireframe )
			gl::enableWireframe();
		if( mUseVbo ) {
			if( mDrawAllSections ) {
				model::Renderer::draw( mSkeletalMesh );
			} else {
				if( mSectionId < mSkeletalMesh->getSections().size() ) {
					model::Renderer::draw( mSkeletalMesh, mSectionId );
				}
			}
		} else {
			model::Renderer::draw( mSkeletalTriMesh );
		}
		if ( mEnableWireframe )
			gl::disableWireframe();
	}
	
	if( mSkeletalMesh->getSkeleton() ) {
		if( mDrawSkeleton) {
			model::Renderer::draw( mSkeletalMesh->getSkeleton() );
		}
		
		if( mDrawLabels ) {
			model::Renderer::drawLabels( mSkeletalMesh->getSkeleton(),  mCamera );
		}
		
		if( mBoundingBox ) {
			const auto& box = mSkeletalMesh->getSkeleton()->calcBoundingBox();
			gl::enableWireframe();
			gl::drawColorCube( box.getCenter(), box.getSize() );
			gl::disableWireframe();
		}
	}
	
	mParams.draw();
}

CINDER_APP( SeymourDemo, RendererGl )
