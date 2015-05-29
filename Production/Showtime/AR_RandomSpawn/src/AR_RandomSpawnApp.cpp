#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AR_RandomSpawnApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void AR_RandomSpawnApp::setup()
{
}

void AR_RandomSpawnApp::mouseDown( MouseEvent event )
{
}

void AR_RandomSpawnApp::update()
{
}

void AR_RandomSpawnApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( AR_RandomSpawnApp, RendererGl )
