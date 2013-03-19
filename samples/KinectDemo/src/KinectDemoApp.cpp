#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class KinectDemoApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void KinectDemoApp::setup()
{
}

void KinectDemoApp::mouseDown( MouseEvent event )
{
}

void KinectDemoApp::update()
{
}

void KinectDemoApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( KinectDemoApp, RendererGl )
