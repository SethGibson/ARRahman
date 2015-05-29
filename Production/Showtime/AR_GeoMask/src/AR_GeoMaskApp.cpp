#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AR_GeoMaskApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void AR_GeoMaskApp::setup()
{
}

void AR_GeoMaskApp::mouseDown( MouseEvent event )
{
}

void AR_GeoMaskApp::update()
{
}

void AR_GeoMaskApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( AR_GeoMaskApp, RendererGl )
