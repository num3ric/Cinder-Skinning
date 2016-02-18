#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/Utilities.h"

#include "InverseKinematics.h"
#include "model/Node.h"
#include "model/Renderer.h"

using namespace ci;

/* References:
 * - Computer Animation- algorithms and techniques 2012. Chapter 5.3.
 * - http://math.ucsd.edu/~sbuss/ResearchWeb/ikmethods/iksurvey.pdf
 * - http://graphics.ucsd.edu/courses/cse169_w04/ (quaternion joints)
 */

class IK3dApp : public app::App {
public:
	void setup() override;
	void resize() override;
	void keyDown( app::KeyEvent event ) override;
	void mouseMove( app::MouseEvent event ) override;
	void mouseDown( app::MouseEvent event ) override;
	void mouseDrag( app::MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	int mNumJoints;
private:
	void generateChain( size_t numJoints );
	
	params::InterfaceGlRef		mParams;
	std::vector<model::NodeRef>	mJoints;
	model::SkeletonRef			mSkeleton;
	
	bool	mRunSolver;
	int		mNumIterations;
	
	CameraPersp	mCamera;
	CameraUi	mCamUi;
	ik::Solver	mIKSolver;
};

void IK3dApp::generateChain( size_t numJoints )
{
	mJoints.clear();
	auto axis = vec3( 0, 1, 0 );
	auto rot = angleAxis( float( 0.5 * (Rand::randFloat()+1.0f) * M_PI ), axis );
	model::NodeRef root = model::Node::create( vec3( 0 ), rot, vec3( 1 ), "0" );
	mJoints.emplace_back( root );
	for( size_t i = 1; i < numJoints; ++i ) {
		auto planePos = ( Rand::randFloat() * 25.0f + 10.0f ) * Rand::randVec2();
		auto pos = vec3( planePos.x, 0.0f, planePos.y );
		//axis[(i+1) % 3] = 1.0f;
		model::NodeRef joint = model::Node::create( pos, angleAxis( float( 0.5 * (Rand::randFloat()+1.0f) * M_PI ), axis ), vec3( 1 ), toString( i ), mJoints.back() );
		mJoints.back()->addChild( joint );
		mJoints.emplace_back( joint );
	}
	
	mSkeleton = model::Skeleton::create( mJoints.front() );
}

void IK3dApp::setup()
{
	Rand::randomize();
	mRunSolver = true;
	
	mNumJoints = 4;
	mParams = params::InterfaceGl::create( "Params", ivec2( 150, 300 ) );
	mParams->addParam( "Run solver", &mRunSolver );
	mNumIterations = 10;
	mParams->addParam( "Num solve iterations", &mNumIterations, "min=1" );
	mParams->addParam( "Use DLS method", &mIKSolver.mUseDLS );
	mParams->addParam( "Max step distance", &mIKSolver.mMaxStep, "min=0.0 step=1.0" );
	mParams->addParam( "Num joints", &mNumJoints, "min=1" );
	mParams->addButton( "Regenerate", [this]() {
		generateChain( mNumJoints );
	} );
	
	mParams->addParam( "Use alternate jacobian", &mIKSolver.mUseAlternateJacobian );
	mParams->addSeparator();
	mParams->addText( "Damped least squares method" );
	mParams->addParam( "Damping factor", &mIKSolver.mLambda, "min=0.0" );
	mParams->addSeparator();
	mParams->addText( "Jacobian Transpose" );
	mParams->addParam( "stepsize", &mIKSolver.mStepsize, "step=0.00001" );
	
	generateChain( mNumJoints );
}

void IK3dApp::update()
{
	float angle = 0.1f * getElapsedSeconds();
	float length = 5.0f * (0.5f * (1.0f + sin( angle ) ) + 0.25f);
	auto goal = length * vec3( cos( angle * 2 * M_PI ) * 7, 6 * sin( 0.15f * getElapsedSeconds() * 2 + angle * 4 * M_PI ), sin( angle * 2 * M_PI ) * 11 );
	mIKSolver.setGoal( goal );
	
	if( mRunSolver ) {
		auto node = mJoints.front();
		std::vector<model::NodeRef> chain;
		while( node->hasChildren() ) {
			chain.emplace_back( node );
			node = node->getChildren().at( 0 );
		}
		
		for( int i=0; i<mNumIterations; ++i ) {
			// node is now the end effector
			mIKSolver.solve( chain, node );
		}
	}
}

void IK3dApp::draw()
{
	gl::clear();
	gl::scale( 0.2, 0.2, 0.2 );
	gl::setMatrices( mCamera );
	
	gl::color( Color( 0, 0, 1 ) );
	gl::drawSphere( mIKSolver.getGoal(), 2.0f, 5 );
	
	gl::color( Color( 0.5, 0.5, 1 ) );
	gl::enableWireframe();
	gl::drawCube( vec3( 0 ), vec3( 100 ) );
	gl::drawSphere( mIKSolver.getClampedGoal(), 3.0f, 4 );
	gl::disableWireframe();
	
	model::Renderer::draw( mSkeleton );
	
	gl::color( Color( 1, 0, 0 ) );
	for( auto& joint : mJoints ) {
		gl::drawLine( joint->getAbsolutePosition(), joint->getAbsolutePosition() + 5.0f * axis( joint->getAbsoluteRotation() ) );
	}
	
	mParams->draw();
}


void IK3dApp::keyDown( app::KeyEvent event )
{
	if( event.getCode() == app::KeyEvent::KEY_f ) {
		app::setFullScreen( !isFullScreen() );
	}
	else if( event.getCode() == app::KeyEvent::KEY_ESCAPE ) {
		quit();
	}
}

void IK3dApp::mouseMove( app::MouseEvent event )
{
	
}

void IK3dApp::mouseDown( app::MouseEvent event )
{
	mCamUi.mouseDown( event.getPos() );
}

void IK3dApp::mouseDrag( app::MouseEvent event )
{
	// Added support for international mac laptop keyboards.
	bool middle = event.isMiddleDown() || ( event.isMetaDown() && event.isLeftDown() );
	bool right = event.isRightDown() || ( event.isControlDown() && event.isLeftDown() );
	mCamUi.mouseDrag( event.getPos(), event.isLeftDown() && !middle && !right, middle, right );
}

void IK3dApp::resize()
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
	mCamera.setEyePoint( vec3(0,0,-150.0f) );
	mCamera.lookAt( vec3( 0 ) );
	mCamera.setFov( 50.0f );
	mCamUi.setWindowSize( getWindowSize() );
	gl::enableDepthRead();
}

CINDER_APP( IK3dApp, app::RendererGl )
