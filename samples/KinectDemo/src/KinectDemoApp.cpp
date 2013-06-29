#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/Utilities.h"

#include "Kinect.h"
#include "Node.h"
#include "Skeleton.h"
#include "ModelSourceAssimp.h"

#include <string>

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace model;

//TODO: Update & simplify later when I have access to a Kinect!

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
	std::vector<KinectSdk::Skeleton>	mKinectSkeletons;
	void								onSkeletonData( std::vector<KinectSdk::Skeleton> skeletons, 
		const KinectSdk::DeviceOptions &deviceOptions );

	// Camera
	ci::CameraPersp						mCamera;
	Colorf								mColor;

	void createKinectSkeleton( const KinectSdk::Skeleton& kSkeleton );

	//
	SkeletonRef mSkeleton;
};

void KinectDemoApp::setup()
{
	mSkeleton = nullptr;
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

void KinectDemoApp::createKinectSkeleton( const KinectSdk::Skeleton& kSkeleton )
{
//	assert( kSkeleton.size() == 20 && mSkeleton->getNumBones() == 0 );
//	
//	// First pass, fill all the names matching the joint indices
//	for( const auto& entry : kSkeleton ) {
//		const KinectSdk::Bone& kBone = entry.second;
//		std::string id = std::to_string( kBone.getEndJoint() );
//		NodeRef newNode = make_shared<Node>( Matrix44f::identity(), Matrix44f::identity(), id );
//		mSkeleton->insertBone( id, newNode );
//	}
//
//	// Second pass, construct the hierarchy and set the root node
//	for( const auto& entry : kSkeleton ) {
//		const KinectSdk::Bone& kBone = entry.second;
//		std::string id = std::to_string( kBone.getEndJoint() );
//		std::string parentId = std::to_string( kBone.getStartJoint() );
//		if( id != parentId ) {
//			try {
//				NodeRef child = mSkeleton->getBone( id );
//				NodeRef parent = mSkeleton->getBone( parentId );
//				child->setParent( parent );
//				parent->addChild( child );
//				
//			} catch ( const std::out_of_range& ) {
//				app::console() << "No parent:" << parentId << std::endl;
//			}
//		} else if ( id == "0" ) {
//			mSkeleton->setRootNode( mSkeleton->getBone( id ) );
//		}
//	}
}

void KinectDemoApp::update()
{
	if ( mKinect->isCapturing() ) {
		mKinect->update();
//		if( mKinectSkeletons.size() > 0 ) {
//			const KinectSdk::Skeleton& kSkeleton = *mKinectSkeletons.begin();
//			if( kSkeleton.size() == 20 ) {
//				if( mSkeleton != nullptr ) {
//					for(auto entry : kSkeleton ) {
//						const KinectSdk::Bone& kBone = entry.second;
//						NodeRef bone = mSkeleton->getBone( std::to_string( kBone.getEndJoint() ) );
//						Vec3f pos = kBone.getPosition();
//						pos.z *= -1.0f;
//						bone->setAbsolutePosition( pos );
//					}
//					int i = 0;
//				} else { 
//					app::console() << "Skeleton created." << std::endl;
//					mSkeleton = Skeleton::create();
//					createKinectSkeleton( kSkeleton );
//				}
//			} else {
//				app::console() << "Missing bones." << std::endl;
//			}
//		}

		mColor = mKinect->getUserColor( 0 );

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
		gl::color( mColor );

//		if( mSkeleton ) {
//			mSkeleton->draw();
//		}
	}
}

// Receives skeleton data
void KinectDemoApp::onSkeletonData( vector<KinectSdk::Skeleton> skeletons, const KinectSdk::DeviceOptions &deviceOptions )
{
	mKinectSkeletons = skeletons;
}

void KinectDemoApp::shutdown()
{
	// Stop input
	mKinect->removeCallback( mCallbackId );
	mKinect->stop();
}

CINDER_APP_NATIVE( KinectDemoApp, RendererGl )
