#ifdef _DEBUG
#pragma comment(lib, "DSAPI32.dbg.lib")
#else
#pragma comment(lib, "DSAPI32.lib")
#endif

#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/ImageIo.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class ChromeCloudApp : public AppNative
{
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag(MouseEvent event) override;
	void update() override;
	void draw() override;

	void exit();

	struct CloudPoint
	{
		vec3 IPosition;
		vec3 IColor;
		CloudPoint(vec3 pPos, vec3 pColor) : IPosition(pPos), IColor(pColor){}
	};

private:
	void setupDSAPI();
	void setupMeshes();
	void setupFBOs();

	//Skybox
	geom::Cube				mSkyBox;
	gl::BatchRef			mSkyBoxBatch;
	gl::TextureCubeMapRef	mTexCube;
	gl::GlslProgRef			mSkyBoxShader;
	gl::FboRef				mSkyboxFbo;

	//PointCloud
	geom::Sphere			mCloudCube;
	gl::BatchRef			mCloudBatch;
	geom::BufferLayout		mCloudAttribs;
	gl::VboRef				mCloudData;
	gl::VboMeshRef			mCloudMesh;
	gl::GlslProgRef			mCloudShader;
	gl::FboRef				mCloudFbo;

	CameraPersp				mCamera;
	MayaCamUI				mMayaCam;
	vector<CloudPoint>		mPositions;


	CinderDSRef				mCinderDS;
	ivec2					mDepthDims;
};

void ChromeCloudApp::setup()
{
	getWindow()->setSize(1280, 720);

	setupDSAPI();
	setupMeshes();
	setupFBOs();

	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 0.1, 2000.0f);
	mCamera.lookAt(vec3(0, 0, -3), vec3(0), vec3(0, 1, 0));
	mCamera.setCenterOfInterestPoint(vec3(0));
	mMayaCam.setCurrentCam(mCamera);

	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void ChromeCloudApp::setupDSAPI()
{
	mCinderDS = CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHSD, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);
	mDepthDims = mCinderDS->getDepthSize();
	mCinderDS->start();

	getSignalShutdown().connect(std::bind(&ChromeCloudApp::exit, this));
}

void ChromeCloudApp::setupMeshes()
{
	try
	{
		mSkyBoxShader = gl::GlslProg::create(loadAsset("sky_box.vert"), loadAsset("sky_box.frag"));
	}
	catch (const gl::GlslProgExc &e)
	{
		console() << e.what() << endl;
	}

	mTexCube = gl::TextureCubeMap::create(loadImage(loadAsset("env_map.jpg")), gl::TextureCubeMap::Format().mipmap());
	mSkyBox = geom::Cube();
	mSkyBoxBatch = gl::Batch::create(mSkyBox, mSkyBoxShader);
	mSkyBoxBatch->getGlslProg()->uniform("uCubeMapTex", 0);

	try
	{
		mCloudShader = gl::GlslProg::create(loadAsset("instcloud_vert.glsl"), loadAsset("instcloud_frag.glsl"));
	}
	catch (const gl::GlslProgExc &e)
	{
		console() << e.what() << endl;
	}
	mPositions.clear();

	for (int y = 0; y < mDepthDims.y; ++y)
	{
		for (int x = 0; x < mDepthDims.x; ++x)
		{
			float cx = lmap<float>(x, 0, mDepthDims.x, -2.666f, 2.666f);
			float cy = lmap<float>(y, 0, mDepthDims.y, -2.0f, 2.0f);
			mPositions.push_back(CloudPoint(vec3(cx, cy, 0),vec3(1)));
		}
	}

	mCloudCube = geom::Sphere().radius(0.01f).subdivisions(8);
	mCloudMesh = gl::VboMesh::create(mCloudCube);

	mCloudData = gl::Vbo::create(GL_ARRAY_BUFFER, mPositions, GL_DYNAMIC_DRAW);
	mCloudAttribs.append(geom::CUSTOM_0, 3, sizeof(CloudPoint), offsetof(CloudPoint, IPosition), 1);
	mCloudAttribs.append(geom::CUSTOM_1, 3, sizeof(CloudPoint), offsetof(CloudPoint, IColor), 1);
	
	mCloudMesh->appendVbo(mCloudAttribs, mCloudData);
	mCloudBatch = gl::Batch::create(mCloudMesh, mCloudShader, { { geom::CUSTOM_0, "iPosition" }, { geom::CUSTOM_1, "iColor" } });
	mCloudBatch->getGlslProg()->uniform("uCubeMapTex", 0);
}

void ChromeCloudApp::setupFBOs()
{
	mCloudFbo = gl::Fbo::create(1280, 720, true);
}

void ChromeCloudApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown(event.getPos());
}

void ChromeCloudApp::mouseDrag(MouseEvent event)
{
	mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), false, event.isRightDown());
}

void ChromeCloudApp::update()
{
	mCinderDS->update();
	mPositions.clear();
	Channel16u cChanDepth = mCinderDS->getDepthFrame();
	Channel16u::Iter cIter = cChanDepth.getIter();

	while (cIter.line())
	{
		while (cIter.pixel())
		{
			if (cIter.x() % 2 == 0 && cIter.y() % 2 == 0)
			{
				float cVal = (float)cIter.v();
				if (cVal > 100 && cVal < 1000)
				{
					float cx = lmap<float>(cIter.x(), 0, mDepthDims.x, -8.f, 8.f);
					float cy = lmap<float>(cIter.y(), 0, mDepthDims.y, 6.f, -6.f);
					float cz = lmap<float>(cVal, 100.0f, 1000.0f, -8.f, 8.f);

					Color cColor = mCinderDS->getDepthSpaceColor(vec3(cIter.x(), cIter.y(), cVal));
					mPositions.push_back(CloudPoint(vec3(cx, cy, cz),vec3(cColor.r, cColor.g, cColor.b)));
				}
			}
		}
	}

	mCloudData->bufferData(mPositions.size()*sizeof(vec3), mPositions.data(), GL_DYNAMIC_DRAW);
	mCloudMesh = gl::VboMesh::create(mCloudCube);
	mCloudMesh->appendVbo(mCloudAttribs, mCloudData);
	mCloudBatch->replaceVboMesh(mCloudMesh);


}

void ChromeCloudApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatrices(mMayaCam.getCamera());
	mCloudBatch->drawInstanced(mPositions.size());
	
	gl::pushMatrices();
	gl::scale(500, 500, 500);
	mTexCube->bind();
	mSkyBoxBatch->draw();
	gl::popMatrices();
}

void ChromeCloudApp::exit()
{
	mCinderDS->stop();
}

CINDER_APP_NATIVE( ChromeCloudApp, RendererGl )
