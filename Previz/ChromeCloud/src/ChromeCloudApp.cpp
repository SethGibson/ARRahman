#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ChromeCloudApp : public AppNative {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void ChromeCloudApp::setup()
{
}

void ChromeCloudApp::mouseDown( MouseEvent event )
{
}

void ChromeCloudApp::update()
{
}

void ChromeCloudApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( ChromeCloudApp, RendererGl )
