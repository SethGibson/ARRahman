#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BillboardsApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void BillboardsApp::setup()
{
}

void BillboardsApp::mouseDown( MouseEvent event )
{
}

void BillboardsApp::update()
{
}

void BillboardsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( BillboardsApp, RendererGl )
