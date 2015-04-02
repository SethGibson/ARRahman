#ifdef _DEBUG
#pragma comment(lib, "DSAPI.dbg.lib")
#else
#pragma comment(lib, "DSAPI.lib")
#endif
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/GeomIo.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class SpringApp : public App
{
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag(MouseEvent event) override;
	void update() override;
	void draw() override;
	void cleanup() override;

private:
	void setupDS();
	void setupScene();
	void setupFBO();

	void updatePointCloud();
	void updateFBO();

	void drawPointCloud();
	void drawSkyBox();

	//DSAPI
	CinderDSRef			mCinderDS;
	gl::Texture2dRef	mTexRgb;

	//PointCloud
	struct CloudPoint
	{
		vec3 PPosition;
		vec2 PUV;
		CloudPoint(vec3 pPos, vec2 pUV) : PPosition(pPos), PUV(pUV){}
	};

	vector<CloudPoint>	mPointsCloud;
	gl::VboRef			mDataInstance;
	geom::BufferLayout	mAttribsInstance;
	geom::Sphere		mMeshInstance;
	gl::VboMeshRef		mMeshCloud;
	gl::GlslProgRef		mShaderCloud;
	gl::BatchRef		mBatchCloud;
	

	//Skybox
	geom::Cube				mMeshSkyBox;
	gl::TextureCubeMapRef	mTexSkyBox;
	gl::GlslProgRef			mShaderSkyBox;
	gl::BatchRef			mBatchSkyBox;

	CameraPersp				mCamera;
	MayaCamUI				mMayaCam;
};

void SpringApp::setup()
{
	setupDS();
	setupScene();
	setupFBO();

	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void SpringApp::setupDS()
{
	mCinderDS = CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHQVGA, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);
	mCinderDS->start();
	mTexRgb = gl::Texture2d::create(mCinderDS->getRgbWidth(), mCinderDS->getRgbHeight(), gl::Texture2d::Format().internalFormat(GL_RGB8));
}

void SpringApp::setupScene()
{
	getWindow()->setSize(1280, 720);
	setFrameRate(60);

	//Camera
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 1, 5000);
	mCamera.lookAt(vec3(0), vec3(0, 0, 1), vec3(0, 1, 0));
	mCamera.setCenterOfInterestPoint(vec3(0, 0, 150));
	mMayaCam.setCurrentCam(mCamera);

	//PointCloud
	mPointsCloud.clear();
	ivec2 cDims = mCinderDS->getDepthSize();
	for (int dy = 0; dy < cDims.y; ++dy)
	{
		for (int dx = 0; dx < cDims.x; ++dx)
		{
			mPointsCloud.push_back(CloudPoint(vec3(0), vec2(0)));
		}
	}

	mMeshInstance = geom::Sphere().radius(0.5f);
	mMeshCloud = gl::VboMesh::create(mMeshInstance);

	mDataInstance = gl::Vbo::create(GL_ARRAY_BUFFER, mPointsCloud, GL_DYNAMIC_DRAW);
	mAttribsInstance.append(geom::CUSTOM_0, 3, sizeof(CloudPoint), offsetof(CloudPoint, PPosition), 1);
	mAttribsInstance.append(geom::CUSTOM_1, 2, sizeof(CloudPoint), offsetof(CloudPoint, PUV), 1);
	mMeshCloud->appendVbo(mAttribsInstance, mDataInstance);

	mShaderCloud = gl::GlslProg::create(loadAsset("cloud.vert"), loadAsset("cloud.frag"));
	mBatchCloud = gl::Batch::create(mMeshCloud, mShaderCloud, { { geom::CUSTOM_0, "iPosition" }, { geom::CUSTOM_1, "iUV" } });
	mBatchCloud->getGlslProg()->uniform("mTexRgb", 0);
	mBatchCloud->getGlslProg()->uniform("mTexCube", 1);

	//Skybox
	mTexSkyBox = gl::TextureCubeMap::create(loadImage(loadAsset("ph_cubemap.png")), gl::TextureCubeMap::Format().mipmap().internalFormat(GL_RGBA8));
	mShaderSkyBox = gl::GlslProg::create(loadAsset("skybox.vert"), loadAsset("skybox.frag"));
	mBatchSkyBox = gl::Batch::create(geom::Cube(), mShaderSkyBox);
	mBatchSkyBox->getGlslProg()->uniform("uCubeMapTex", 0);
}

void SpringApp::setupFBO()
{
}

void SpringApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown(event.getPos());
}

void SpringApp::mouseDrag(MouseEvent event)
{
	mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), false, event.isRightDown());
}

void SpringApp::update()
{
	updatePointCloud();
}

void SpringApp::updatePointCloud()
{
	mCinderDS->update();
	mTexRgb->update(mCinderDS->getRgbFrame());

	Channel16u cDepth = mCinderDS->getDepthFrame();
	Channel16u::Iter cIter = cDepth.getIter();

	mPointsCloud.clear();
	while (cIter.line())
	{
		while (cIter.pixel())
		{
			float cVal = (float)cIter.v();
			if (cVal > 0 && cVal < 1000)
			{
				float cX = cIter.x();
				float cY = cIter.y();
				vec3 cWorld = mCinderDS->getZCameraSpacePoint(vec3(cX, cY, cVal));
				vec2 cUV = mCinderDS->getColorSpaceCoordsFromZCamera(cWorld);
				mPointsCloud.push_back(CloudPoint(cWorld, cUV));
			}
		}
	}

	mMeshCloud = gl::VboMesh::create(mMeshInstance);
	mDataInstance->bufferData(mPointsCloud.size()*sizeof(CloudPoint), mPointsCloud.data(), GL_DYNAMIC_DRAW);
	mMeshCloud->appendVbo(mAttribsInstance, mDataInstance);
	mBatchCloud->replaceVboMesh(mMeshCloud);

}

void SpringApp::updateFBO()
{

}

void SpringApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatrices(mMayaCam.getCamera());

	drawSkyBox();
	drawPointCloud();
}

void SpringApp::drawSkyBox()
{
	gl::pushMatrices();
	gl::scale(2000, 2000, 2000);
	mTexSkyBox->bind();
	mBatchSkyBox->draw();
	mTexSkyBox->unbind();
	gl::popMatrices();
}

void SpringApp::drawPointCloud()
{
	mBatchCloud->getGlslProg()->uniform("ViewDirection", mMayaCam.getCamera().getViewDirection());
	mBatchCloud->getGlslProg()->uniform("LightPosition", vec3(0));
	gl::pushMatrices();
	gl::scale(vec3(1, -1, 1));
	mTexRgb->bind(0);
	mTexSkyBox->bind(1);
	mBatchCloud->drawInstanced(mPointsCloud.size());
	mTexRgb->unbind();
	mTexSkyBox->unbind();
	gl::popMatrices();
}

void SpringApp::cleanup()
{
	mCinderDS->stop();
}

CINDER_APP( SpringApp, RendererGl )
