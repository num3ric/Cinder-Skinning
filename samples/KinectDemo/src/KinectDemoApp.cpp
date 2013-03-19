#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"

#include "Kinect.h"
#include "Skeleton.h"
#include "ModelSourceAssimp.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace KinectSdk;

class KinectDemoApp : public AppNative {
  public:
	void	setup();
	void	mouseDown( MouseEvent event );	
	void	update();
	void	draw();
	void	shutdown();
private:
	// Kinect
	uint32_t							mCallbackId;
	KinectSdk::KinectRef				mKinect;
	std::vector<KinectSdk::Skeleton>	mSkeletons;
	void								onSkeletonData( std::vector<KinectSdk::Skeleton> skeletons, 
		const KinectSdk::DeviceOptions &deviceOptions );

	// Camera
	ci::CameraPersp						mCamera;
};

void KinectDemoApp::setup()
{
	// Start Kinect
	mKinect = KinectSdk::Kinect::create();
	mKinect->start( KinectSdk::DeviceOptions().enableDepth( false ).enableColor( false ) );
	mKinect->removeBackground();

	// Set the skeleton smoothing to remove jitters. Better smoothing means
	// less jitters, but a slower response time.
	mKinect->setTransform( KinectSdk::Kinect::TRANSFORM_SMOOTH );

	// Add callback to receive skeleton data
	mCallbackId = mKinect->addSkeletonTrackingCallback( &KinectDemoApp::onSkeletonData, this );

	// Set up camera
	mCamera.lookAt( Vec3f( 0.0f, 0.0f, 2.0f ), Vec3f::zero() );
	mCamera.setPerspective( 45.0f, getWindowAspectRatio(), 0.01f, 1000.0f );
}

void KinectDemoApp::mouseDown( MouseEvent event )
{
}

void KinectDemoApp::update()
{
	if ( mKinect->isCapturing() ) {
		mKinect->update();
	} else {
		// If Kinect initialization failed, try again every 90 frames
		if ( getElapsedFrames() % 90 == 0 ) {
			mKinect->start();
		}
	}
}

void KinectDemoApp::draw()
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( Colorf::gray( 0.1f ) );

	// We're capturing
	if ( mKinect->isCapturing() ) {

		// Set up 3D view
		gl::setMatrices( mCamera );

		// Iterate through skeletons
		uint32_t i = 0;
		for ( vector<KinectSdk::Skeleton>::const_iterator skeletonIt = mSkeletons.cbegin(); skeletonIt != mSkeletons.cend(); ++skeletonIt, i++ ) {

			// Set color
			Colorf color = mKinect->getUserColor( i );

			// Iterate through joints
			for ( KinectSdk::Skeleton::const_iterator boneIt = skeletonIt->cbegin(); boneIt != skeletonIt->cend(); ++boneIt ) {

				// Set user color
				gl::color( color );

				// Get position and rotation
				const Bone& bone	= boneIt->second;
				Vec3f position		= bone.getPosition();
				Matrix44f transform	= bone.getAbsoluteRotationMatrix();
				Vec3f direction		= transform.transformPoint( position ).normalized();
				direction			*= 0.05f;
				position.z			*= -1.0f;

				// Draw bone
				glLineWidth( 2.0f );
				JointName startJoint = bone.getStartJoint();
				if ( skeletonIt->find( startJoint ) != skeletonIt->end() ) {
					Vec3f destination	= skeletonIt->find( startJoint )->second.getPosition();
					destination.z		*= -1.0f;
					gl::drawLine( position, destination );
				}

				// Draw joint
				gl::drawSphere( position, 0.025f, 16 );

				// Draw joint orientation
				glLineWidth( 0.5f );
				gl::color( ColorAf::white() );
				gl::drawVector( position, position + direction, 0.05f, 0.01f );

			}

		}

	}
}

// Receives skeleton data
void KinectDemoApp::onSkeletonData( vector<KinectSdk::Skeleton> skeletons, const DeviceOptions &deviceOptions )
{
	mSkeletons = skeletons;
}

void KinectDemoApp::shutdown()
{
	// Stop input
	mKinect->removeCallback( mCallbackId );
	mKinect->stop();
}

CINDER_APP_NATIVE( KinectDemoApp, RendererGl )
