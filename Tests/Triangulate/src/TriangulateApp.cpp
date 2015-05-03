#ifdef _DEBUG
#pragma comment(lib, "DSAPI.dbg.lib")
#else
#pragma comment(lib, "DSAPI.lib")
#endif

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "CiDelaunay.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TriangulateApp : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void cleanup() override;

private:
	void setupBatch();
	void setupDSAPI();

	CinderDS::CinderDSRef	mCinderDS;
	ivec2					mDepthSize;

	vector<vec3>			mPoints;
	TriMeshRef				mTriangles;
	gl::VboRef				mVbo;
	gl::BatchRef			mBatch;
	geom::BufferLayout		mAttribs;

	gl::GlslProgRef			mShader;
	string					mVertShader, mFragShader;

	CameraPersp				mCamera;
	CameraUi				mCamUI;
};

void TriangulateApp::setup()
{
	getWindow()->setSize(1280, 720);
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 100.0f, 3000.0f);
	mCamera.lookAt(vec3(0), vec3(0, 0, 1), vec3(0, -1, 0));
	mCamera.setPivotDistance(750.0f);
	mCamUI = CameraUi(&mCamera, getWindow());

	setupDSAPI();
	setupBatch();
}

void TriangulateApp::update()
{
	mCinderDS->update();
	mPoints.clear();
	const uint16_t *cDepth = mCinderDS->getDepthFrame().getData();

	for (int dy = 0; dy < mDepthSize.y; dy += 6)
	{
		for (int dx = 0; dx < mDepthSize.x; dx += 8)
		{
			float cVal = (float)cDepth[dy*mDepthSize.x + dx];
			if (cVal>100 && cVal < 1500)
			{
				mPoints.push_back(mCinderDS->getDepthSpacePoint(static_cast<float>(dx), static_cast<float>(dy), cVal));
			}
		}
	}

	mVbo->bufferData(mPoints.size()*sizeof(vec3), mPoints.data(), GL_DYNAMIC_DRAW);
	gl::VboMeshRef cMesh = gl::VboMesh::create(mPoints.size(), GL_POINTS, { {mAttribs, mVbo} });
	mBatch->replaceVboMesh(cMesh);

	if (mPoints.size()>3)
		mTriangles = CiDelaunay::triangulate(mPoints);
}

void TriangulateApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatrices(mCamera);

	gl::enableWireframe();
	gl::color(Color::white());

	if (mTriangles)
		gl::draw(*mTriangles);
	gl::disableWireframe();

	mBatch->draw();
}

void TriangulateApp::setupBatch()
{
	mVertShader =
		"uniform mat4 ciModelViewProjection;"
		"in vec4 ciPosition;"
		"void main(){"
		"gl_Position=ciModelViewProjection*ciPosition; }";
	mFragShader =
		"out vec4 Color;"
		"void main(){"
		"Color=vec4(1,1,1,1); }";

	mShader = gl::GlslProg::create(mVertShader, mFragShader);

	for (int dy = 0; dy < mDepthSize.y; dy+=6)
	{
		for (int dx = 0; dx < mDepthSize.x; dx+=8)
		{
			mPoints.push_back(vec3(dx, dy, 0));
		}
	}

	mVbo = gl::Vbo::create(GL_ARRAY_BUFFER, mPoints, GL_DYNAMIC_DRAW);
	mAttribs.append(geom::POSITION,3,0,0,0);

	gl::VboMeshRef cMesh = gl::VboMesh::create(mPoints.size(), GL_POINTS, { {mAttribs, mVbo} });
	mBatch = gl::Batch::create(cMesh, mShader);
}

void TriangulateApp::setupDSAPI()
{
	mCinderDS = CinderDS::CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(CinderDS::FrameSize::DEPTHQVGA, 60);
	mCinderDS->start();
	mDepthSize = mCinderDS->getDepthSize();
}

void TriangulateApp::cleanup()
{
	mCinderDS->stop();
}

CINDER_APP( TriangulateApp, RendererGl )
