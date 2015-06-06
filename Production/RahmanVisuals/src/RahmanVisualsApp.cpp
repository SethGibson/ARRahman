#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class RahmanVisualsApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void RahmanVisualsApp::setup()
{
}

void RahmanVisualsApp::mouseDown( MouseEvent event )
{
}

void RahmanVisualsApp::update()
{
}

void RahmanVisualsApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( RahmanVisualsApp, RendererGl )
