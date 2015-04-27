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
#include "cinder/Rand.h"
#include "CiDSAPI.h"
#include "ParticleController.h"
#include "Particle.h"


using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class WinterApp : public App
{
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
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

	//Particle controller
	ParticleController mParticleController;
	int mNumberOfParticles;
	int mNoOfFramesBeforeSpawingParticles;
	int mNoOfFramesElapsed;

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

	//Snow
	vector<Particle>	mPointsSnow;
	gl::VboRef			mSnowDataInstance;
	geom::BufferLayout	mSnowAttribsInstance;
	geom::Sphere		mSnowMeshInstance;
	gl::VboMeshRef		mMeshSnow;
	gl::GlslProgRef		mShaderSnow;
	gl::BatchRef		mBatchSnow;


	//Skybox
	geom::Cube				mMeshSkyBox;
	gl::TextureCubeMapRef	mTexSkyBox;
	gl::GlslProgRef			mShaderSkyBox;
	gl::BatchRef			mBatchSkyBox;

	CameraPersp				mCamera;
	MayaCamUI				mMayaCam;

	
};

void WinterApp::setup()
{
	mNoOfFramesBeforeSpawingParticles = 200;
	mNumberOfParticles = 20000;
	mNoOfFramesElapsed = 200;
	
	setupDS();
	setupScene();
	setupFBO();

	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void WinterApp::setupDS()
{
	mCinderDS = CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHQVGA, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);
	mCinderDS->start();
	mTexRgb = gl::Texture2d::create(mCinderDS->getRgbWidth(), mCinderDS->getRgbHeight(), gl::Texture2d::Format().internalFormat(GL_RGB8));
}

void WinterApp::setupScene()
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
	
	//snow
	mPointsSnow.clear();
	//for (int i = 0; i<mNumberOfParticles; i++)
	//{

	//	vec3 loc = vec3((float)Rand::randInt(320), (float)Rand::randInt(240), (float)Rand::randInt(0, 1000));
	//	//vec2 loc = mouseLoc + Rand::randVec2f() * Rand::randFloat(5.0f);
	//	//vec2 velOffset = Rand::randVec2f() * Rand::randFloat(1.0f, 3.0f);
	//	//vec2 vel = mouseVel * 5.0f + velOffset;
	//	//vec3 vel = Rand::randVec3f();//change this for the falling behavior of the snow
	//	mPointsSnow.push_back(Particle(loc));
	//}

	

	mSnowDataInstance = gl::Vbo::create(GL_ARRAY_BUFFER, mPointsSnow, GL_DYNAMIC_DRAW);
	mSnowAttribsInstance.append(geom::CUSTOM_4, 3, sizeof(Particle), offsetof(Particle, mLoc), 1);
	

	mSnowMeshInstance = geom::Sphere().radius(0.2f);
	mMeshSnow = gl::VboMesh::create(mSnowMeshInstance);
	mMeshSnow->appendVbo(mSnowAttribsInstance, mSnowDataInstance);

	mShaderSnow = gl::GlslProg::create(loadAsset("snow.vert"), loadAsset("snow.frag"));
	mBatchSnow = gl::Batch::create(mMeshSnow, mShaderSnow, { { geom::CUSTOM_4, "iPosition" } });
	mBatchSnow->getGlslProg()->uniform("mTexCube", 0);

	//Skybox
	mTexSkyBox = gl::TextureCubeMap::create(loadImage(loadAsset("ph_cubemap.png")), gl::TextureCubeMap::Format().mipmap().internalFormat(GL_RGBA8));
	mShaderSkyBox = gl::GlslProg::create(loadAsset("skybox.vert"), loadAsset("skybox.frag"));
	mBatchSkyBox = gl::Batch::create(geom::Cube(), mShaderSkyBox);
	mBatchSkyBox->getGlslProg()->uniform("mTexCube", 0);
}

void WinterApp::setupFBO()
{
}

void WinterApp::mouseDown(MouseEvent event)
{
	mMayaCam.mouseDown(event.getPos());
}

void WinterApp::mouseDrag(MouseEvent event)
{
	mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), false, event.isRightDown());
}

void WinterApp::update()
{

	updatePointCloud();

	
	for (int sp = 0; sp < mNumberOfParticles; ++sp)
	{
		if (mPointsSnow.size() < mNumberOfParticles)
		{
			vec3 loc = vec3((float)Rand::randInt(-400, 400), (float)Rand::randInt(-300, -100), (float)Rand::randInt(300, 1200));
				mPointsSnow.push_back(Particle(loc));
		}
	}

	for (vector<Particle>::iterator p = mPointsSnow.begin(); p != mPointsSnow.end();){
		if (p->mIsDead){
			p = mPointsSnow.erase(p);
		}
		else {
			p->mLoc += p->mDir*p->mVelocity;
			p->mAge++;
			if (p->mAge > p->mLifespan) p->mIsDead = true;
			++p;
		}
	}
	
	mSnowDataInstance->bufferData(mPointsSnow.size()*sizeof(Particle), mPointsSnow.data(), GL_DYNAMIC_DRAW);
	mMeshSnow = gl::VboMesh::create(mSnowMeshInstance);
	mMeshSnow->appendVbo(mSnowAttribsInstance, mSnowDataInstance);
	mBatchSnow->replaceVboMesh(mMeshSnow);




}

void WinterApp::updatePointCloud()
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

void WinterApp::updateFBO()
{

}

void WinterApp::draw()
{

	gl::clear(Color(0, 0, 0));
	gl::setMatrices(mMayaCam.getCamera());
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::enableAlphaBlending();
	mParticleController.draw();
	drawSkyBox();
	drawPointCloud();


	//mBatchSnow->getGlslProg()->uniform("ViewDirection", mMayaCam.getCamera().getViewDirection());
	//mBatchSnow->getGlslProg()->uniform("LightPosition", vec3(0));
	gl::pushMatrices();
	gl::scale(vec3(1, -1, 1));
	mTexSkyBox->bind(0);
	mBatchSnow->drawInstanced(mPointsSnow.size());
	mTexSkyBox->unbind();
	gl::popMatrices();
	gl::disableAlphaBlending();

	
}

void WinterApp::drawSkyBox()
{
	gl::pushMatrices();
	gl::scale(2000, 2000, 2000);
	mTexSkyBox->bind();
	mBatchSkyBox->draw();
	mTexSkyBox->unbind();
	gl::popMatrices();

}

void WinterApp::drawPointCloud()
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

void WinterApp::cleanup()
{
	mCinderDS->stop();
}

CINDER_APP(WinterApp, RendererGl)
