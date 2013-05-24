#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CustomShaderDemo : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void CustomShaderDemo::setup()
{
}

void CustomShaderDemo::mouseDown( MouseEvent event )
{
}

void CustomShaderDemo::update()
{
}

void CustomShaderDemo::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( CustomShaderDemo, RendererGl )
