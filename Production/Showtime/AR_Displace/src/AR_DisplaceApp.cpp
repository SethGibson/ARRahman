#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AR_DisplaceApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void AR_DisplaceApp::setup()
{
}

void AR_DisplaceApp::mouseDown( MouseEvent event )
{
}

void AR_DisplaceApp::update()
{
}

void AR_DisplaceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( AR_DisplaceApp, RendererGl )
