#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "Cinder-Bullet3D/BulletContext.h"
#include "Cinder-Bullet3D/RigidBody.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class PhysicsTestApp : public App
{
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

private:
	void getTriangles();

	bullet::ContextRef	mContext;
	CinderDSRef			mCinderDS;

	gl::VboMeshRef		mPhysicsMesh;
	gl::VboRef			mPhysicsVbo;
	gl::VboRef			mIndexVbo;
};

void PhysicsTestApp::setup()
{
	mCinderDS = CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHQVGA, 60);

	ivec2 cDepthSize = mCinderDS->getDepthSize();

	for (int vy = 0; vy < cDepthSize.y; ++vy)
	{
		for (int vx = 0; vx < cDepthSize.x; ++vx)
		{
			
		}
	}
}

void PhysicsTestApp::mouseDown( MouseEvent event )
{
}

void PhysicsTestApp::update()
{
	mCinderDS->update();
	getTriangles();
}

void PhysicsTestApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( PhysicsTestApp, RendererGl )
