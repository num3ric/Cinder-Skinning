#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"

#include "Kinect.h"
#include "Node.h"
#include "Skeleton.h"
#include "ModelSourceAssimp.h"
#include "SkinningRenderer.h"

#include <string>

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace model;

class KinectDemoApp : public AppNative {
  public:
	void	setup();
	void	mouseDown( MouseEvent event ) override;
	void	mouseDrag( MouseEvent event ) override;
	void	resize() override;
	void	update();
	void	draw();
	void	shutdown();
private:
	// Kinect
	uint32_t							mCallbackId;
	MsKinect::DeviceRef					mDevice;
	std::vector<MsKinect::Skeleton>		mDeviceSkeletons;
	void								onFrame( MsKinect::Frame frame, const MsKinect::DeviceOptions &deviceOptions );

	ci::CameraPersp                     mCamera;
	params::InterfaceGl					mParams;
	bool								mDrawSDKSkeleton;
	Colorf								mColor;

	void createKinectSkeleton( const MsKinect::Skeleton& kSkeleton );

	ci::Color	mBgColor;
	SkeletonRef mSkeleton;
};

void KinectDemoApp::setup()
{
	mParams = params::InterfaceGl( "Parameters", Vec2i( 200, 250 ) );
	mDrawSDKSkeleton = false;
	mParams.addParam( "Draw SDK skeleton", &mDrawSDKSkeleton );

	mSkeleton = nullptr;

	mDevice = MsKinect::Device::create();
	mDevice->connectEventHandler( &KinectDemoApp::onFrame, this );
	mDevice->start( MsKinect::DeviceOptions().enableDepth( false ).enableColor( false ) );
	mDevice->removeBackground();
	mDevice->setTransform( MsKinect::Device::TRANSFORM_SMOOTH );


	// Set up camera
	mCamera.lookAt( Vec3f( 0.0f, 0.0f, 2.0f ), Vec3f::zero() );
	mCamera.setPerspective( 45.0f, getWindowAspectRatio(), 0.01f, 1000.0f );
}

void KinectDemoApp::resize()
{

}

void KinectDemoApp::mouseDown( MouseEvent event )
{
}

void KinectDemoApp::mouseDrag( MouseEvent event )
{
}

void KinectDemoApp::createKinectSkeleton( const MsKinect::Skeleton& kSkeleton )
{
	assert( kSkeleton.size() == 20 && mSkeleton->getNumBones() == 0 );

	// First pass, fill all the names matching the joint indices
	for( const auto& kv : kSkeleton ) {
		const MsKinect::Bone& kBone = kv.second;
		std::string id = std::to_string( kBone.getEndJoint() );
		app::console() << kBone.getPosition() << std::endl;
		mSkeleton->addBone( id, std::make_shared<Node>( kBone.getPosition(), Quatf::identity(), Vec3f::one(), id ) );
	}

	app::console() << "hierarchy" << endl;
	// Second pass, construct the hierarchy and set the root node
	for( const auto& entry : kSkeleton ) {
		const MsKinect::Bone& kBone = entry.second;
		std::string id = std::to_string( kBone.getEndJoint() );
		std::string parentId = std::to_string( kBone.getStartJoint() );
		if( id != parentId ) {
			NodeRef child = mSkeleton->getBone( id );
			NodeRef parent = mSkeleton->getBone( parentId );
			child->setParent( parent );
			parent->addChild( child );
		} else if ( id == "0" ) {
			mSkeleton->setRootNode( mSkeleton->getBone( id ) );
		}
	}

	app::console() << *mSkeleton << endl;

	int x = 0;
}

void KinectDemoApp::update()
{
	if ( !mDevice->isCapturing() ) {
		// If Kinect initialization failed, try again every 90 frames
		if ( getElapsedFrames() % 90 == 0 ) {
			mDevice->start();
		}
	}
}

void KinectDemoApp::draw()
{
	mBgColor = Color( ColorModel::CM_HSV, 0.0f/360.0f, 1.0f, 0.5f);

	// We're capturing
	if ( mDevice->isCapturing() ) {
		for( auto it = mDeviceSkeletons.begin(); it != mDeviceSkeletons.end(); ++it ) {
			const auto& kSkeleton = *it;
			if( kSkeleton.size() == 20 ) {
				if( mSkeleton ) {
					app::console() << "Updating full skeleton" << std::endl;
					for(auto entry : kSkeleton ) {
						const MsKinect::Bone& kBone = entry.second;
						NodeRef bone = mSkeleton->getBone( std::to_string( kBone.getEndJoint() ) );
						Vec3f pos = kBone.getPosition();
						pos.z *= -1.0f;
						bone->setAbsolutePosition( pos );
					}
					int i = 0;
				} else { 
					mSkeleton = Skeleton::create();
					createKinectSkeleton( kSkeleton );
					app::console() << "Skeleton created." << std::endl;
				}
				mBgColor = Color( ColorModel::CM_HSV, 120.0f/360.0f, 1.0f, 0.5f);
			}
		}
		mColor = mDevice->getUserColor( 0 );

		// Clear window
		gl::setViewport( getWindowBounds() );
		gl::clear( mBgColor );
		// Set up 3D view
		gl::setMatrices( mCamera );
		gl::scale( Vec3f::one() * 50.0f );
		gl::color( mColor );
		if( !mDrawSDKSkeleton ) {
			if( mSkeleton ) {
				SkinningRenderer::draw( mSkeleton );
			}
		} else {
			using namespace MsKinect;
			uint32_t i = 0;
			for ( auto it = mDeviceSkeletons.begin(); it != mDeviceSkeletons.end(); ++it, ++i ) {
				Colorf color = mDevice->getUserColor( i );
				for ( auto boneIt = it->begin(); boneIt != it->end(); ++boneIt ) {
					gl::color( color );
					const Bone& bone	= boneIt->second;
					Vec3f position		= bone.getPosition();
					Matrix44f transform	= bone.getAbsoluteRotationMatrix();
					Vec3f direction		= transform.transformPoint( position ).normalized();
					direction			*= 0.05f;
					position.z			*= -1.0f;

					glLineWidth( 2.0f );
					JointName startJoint = bone.getStartJoint();
					if( it->find( startJoint ) != it->end() ) {
						Vec3f destination	= it->find( startJoint )->second.getPosition();
						destination.z		*= -1.0f;
						gl::drawLine( position, destination );
					}

					gl::drawSphere( position, 0.025f, 16 );

					glLineWidth( 0.5f );
					gl::color( ColorAf::white() );
					gl::drawVector( position, position + direction, 0.05f, 0.01f );
				}
			}
		}
	}
	mParams.draw();
}

void KinectDemoApp::onFrame( MsKinect::Frame frame, const MsKinect::DeviceOptions &deviceOptions )
{
	mDeviceSkeletons = frame.getSkeletons();
}

void KinectDemoApp::shutdown()
{
	// Stop input
	mDevice->stop();
}

CINDER_APP_NATIVE( KinectDemoApp, RendererGl )