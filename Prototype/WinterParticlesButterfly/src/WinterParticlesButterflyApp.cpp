#ifdef _DEBUG
#pragma comment(lib, "DSAPI.dbg.lib")
#else
#pragma comment(lib, "DSAPI.lib")
#endif
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/GeomIo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/ObjLoader.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "CiDSAPI.h"
#include "CloudParticle.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class WinterParticlesButterflyApp : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void cleanup() override;
	void loadObj(const DataSourceRef &dataSource);

private:
	void setupDS();
	void setupScene();
	void setupFBO();

	void updatePointCloud();
	void updateFBO();
	void updateParticles();

	void drawPointCloud();
	void drawSkyBox();
	void drawParticles();

	//DSAPI
	CinderDSRef			mCinderDS;
	gl::Texture2dRef	mTexRgb;

	int mNoOfFramesBeforeSpawingParticles;
	int mNoOfFramesElapsed;
	int mTimeToSpawnParticles;
	bool mThousandParticlesSpawned;
	int mNumberOfParticlesSpawned;
	int mXIterator;
	int mYIterator;
	int mZIterator;
	int mXDecrementer;
	int mYDecrementer;
	int mZDecrementer;
	gl::Texture2dRef mPetalTex;
	Perlin mPerlin;

	vector<CloudPoint>	mPointsCloud;
	gl::VboRef			mDataInstance;
	geom::BufferLayout	mAttribsInstance;
	geom::Sphere		mMeshInstance;
	gl::VboMeshRef		mMeshCloud;
	gl::GlslProgRef		mShaderCloud;
	gl::BatchRef		mBatchCloud;

	//Snow
	vector<CloudParticle>	mPointsSnow;
	gl::VboRef				mSnowDataInstance;
	geom::BufferLayout		mSnowAttribsInstance;
	geom::Plane				mSnowMeshInstance;
	gl::VboMeshRef			mMeshSnow;
	gl::GlslProgRef			mShaderSnow,
							mShaderBlurU,
							mShaderBlurV;
	gl::BatchRef			mBatchSnow;
	ci::TriMeshRef			mTriMeshSnow;
	gl::FboRef				mFboRaw,
							mFboBlurU,
							mFboBlurV;

	//Skybox
	geom::Cube				mMeshSkyBox;
	gl::TextureCubeMapRef	mTexSkyBox;
	gl::GlslProgRef			mShaderSkyBox;
	gl::BatchRef			mBatchSkyBox;

	CameraPersp				mCamera;
	CameraUi				mMayaCam;
};

void WinterParticlesButterflyApp::setup()
{
	mNoOfFramesBeforeSpawingParticles = 200;
	mNoOfFramesElapsed = 200;  // variable to store after how many frames the particles should be spawned. 
	mNumberOfParticlesSpawned = 0;
	mTimeToSpawnParticles = 0;
	mThousandParticlesSpawned = false;
	mXIterator = 0;
	mYIterator = 0;
	mZIterator = 300;

	mXDecrementer = 0;
	mYDecrementer = 0;
	mZDecrementer = 0;

	mPerlin = Perlin();
	
	setupDS();
	setupScene();
	setupFBO();

	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void WinterParticlesButterflyApp::setupDS()
{
	mCinderDS = CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHQVGA, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);
	mCinderDS->start();
	mTexRgb = gl::Texture2d::create(mCinderDS->getRgbWidth(), mCinderDS->getRgbHeight(), gl::Texture2d::Format().internalFormat(GL_RGB8));
}

void WinterParticlesButterflyApp::setupScene()
{
	//getWindow()->setSize(1920, 1080);
	setFullScreen(true);
	setFrameRate(60);

	//Camera
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 1, 5000);
	mCamera.lookAt(vec3(0), vec3(0, 0, 1), vec3(0, 1, 0));
	mCamera.setPivotDistance(150);
	mMayaCam = CameraUi(&mCamera, getWindow());
	

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

	mMeshInstance = geom::Sphere().radius(0.3f);
	mMeshCloud = gl::VboMesh::create(mMeshInstance);

	mDataInstance = gl::Vbo::create(GL_ARRAY_BUFFER, mPointsCloud, GL_DYNAMIC_DRAW);
	mAttribsInstance.append(geom::CUSTOM_0, 3, sizeof(CloudPoint), offsetof(CloudPoint, PPosition), 1);
	mAttribsInstance.append(geom::CUSTOM_1, 2, sizeof(CloudPoint), offsetof(CloudPoint, PUV), 1);
	mMeshCloud->appendVbo(mAttribsInstance, mDataInstance);

	mShaderCloud = gl::GlslProg::create(loadAsset("cloud_vert.glsl"), loadAsset("cloud_frag.glsl"));
	mBatchCloud = gl::Batch::create(mMeshCloud, mShaderCloud, { { geom::CUSTOM_0, "iPosition" }, { geom::CUSTOM_1, "iUV" } }); 

	mBatchCloud->getGlslProg()->uniform("mTexRgb", 0);
	mBatchCloud->getGlslProg()->uniform("mTexCube", 1);
	
	//snow
	mPointsSnow.clear();
	mSnowDataInstance = gl::Vbo::create(GL_ARRAY_BUFFER, mPointsSnow, GL_DYNAMIC_DRAW);
	mSnowAttribsInstance.append(geom::CUSTOM_2, 3, sizeof(CloudParticle), offsetof(CloudParticle, PPosition), 1);
	mSnowAttribsInstance.append(geom::CUSTOM_3, 16, sizeof(CloudParticle), offsetof(CloudParticle, PModelMatrix), 1);
	mSnowAttribsInstance.append(geom::CUSTOM_4, 1, sizeof(CloudParticle), offsetof(CloudParticle, PSize), 1);

	ObjLoader loader(loadAsset("rosepetal.obj"));
	mTriMeshSnow = ci::TriMesh::create(loader);
	mMeshSnow = gl::VboMesh::create(*mTriMeshSnow);

	if (!loader.getAvailableAttribs().count(geom::NORMAL))
		mTriMeshSnow->recalculateNormals();

	mMeshSnow->appendVbo(mSnowAttribsInstance, mSnowDataInstance);

	mShaderSnow = gl::GlslProg::create(loadAsset("snow_vert.glsl"), loadAsset("snow_frag.glsl"));
	mBatchSnow = gl::Batch::create(mMeshSnow, mShaderSnow, { { geom::CUSTOM_2, "iPosition" }, { geom::CUSTOM_3, "iModelMatrix" }, { geom::CUSTOM_4, "iSize" } });
	mPetalTex = gl::Texture2d::create(loadImage(loadAsset("rosepetal.png")));

	//Skybox
	mTexSkyBox = gl::TextureCubeMap::create(loadImage(loadAsset("ph_cubemap.png")), gl::TextureCubeMap::Format().mipmap().internalFormat(GL_RGBA8));
	mShaderSkyBox = gl::GlslProg::create(loadAsset("skybox_vert.glsl"), loadAsset("skybox_frag.glsl"));
	mBatchSkyBox = gl::Batch::create(geom::Cube(), mShaderSkyBox);
	mBatchSkyBox->getGlslProg()->uniform("mTexCube", 0);
}

void WinterParticlesButterflyApp::setupFBO()
{
	mFboRaw = gl::Fbo::create(getWindowWidth(), getWindowHeight(), gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));
	mFboBlurU = gl::Fbo::create(getWindowWidth(), getWindowHeight(), gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));
	mFboBlurV = gl::Fbo::create(getWindowWidth(), getWindowHeight(), gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));

	//we'll set up fbo shaders here too
	mShaderBlurU = gl::GlslProg::create(loadAsset("blur_u_vert.glsl"), loadAsset("blur_frag.glsl"));
	mShaderBlurV = gl::GlslProg::create(loadAsset("blur_v_vert.glsl"), loadAsset("blur_frag.glsl"));
}

void WinterParticlesButterflyApp::update()
{
	updatePointCloud();
	updateParticles();
	updateFBO();
}

void WinterParticlesButterflyApp::updatePointCloud()
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
			if (cIter.x() % 2 == 0 && cIter.y() % 2 == 0)
			{
				float cVal = (float)cIter.v();
				if (cVal > 100 && cVal < 1000)
				{
					float cX = cIter.x();
					float cY = cIter.y();
					vec3 cWorld = mCinderDS->getDepthSpacePoint(vec3(cX, cY, cVal));
					vec2 cUV = mCinderDS->getColorCoordsFromDepthSpace(cWorld);
					mPointsCloud.push_back(CloudPoint(cWorld, cUV));

				}
			}
		}
	}

	mDataInstance->bufferData(mPointsCloud.size()*sizeof(CloudPoint), mPointsCloud.data(), GL_DYNAMIC_DRAW);
}

void WinterParticlesButterflyApp::updateParticles()
{
	mTimeToSpawnParticles--;

	if (mTimeToSpawnParticles <= 0)
	{
		if (!mThousandParticlesSpawned)
		{
			//Change the number in the for loop to generate more particles at a position per for loop iteration. Higher the number denser the particles.
			int noOfParticlesPerSpawn = 10;
			int TotalNumberOfParticles = 200; // set total number of particles spawned here

			for (int i = 0; i < noOfParticlesPerSpawn; i++)
			{
				mPointsSnow.push_back(CloudParticle(vec3(Rand::randFloat(-50.0f, 50.0f), Rand::randFloat(70.0f, 30.0f), 100)));
				mPointsSnow.push_back(CloudParticle(vec3(Rand::randFloat(-50.0f, 50.0f), Rand::randFloat(70.0f, 30.0f), 100)));
			}

			// add the total of number of particles spawned in both for loops here
			mNumberOfParticlesSpawned = mNumberOfParticlesSpawned + 2 * noOfParticlesPerSpawn;

			// change this number for changing the number of particles
			if (mNumberOfParticlesSpawned > TotalNumberOfParticles)
			{

				//reset after spawning
				mNumberOfParticlesSpawned = 0;
				mXIterator = Rand::randInt(-500, -300);
				mZIterator = Rand::randInt(300, 700);
				mZDecrementer = Rand::randInt(700, 1000);
				mXDecrementer = Rand::randInt(400, 500);
				mThousandParticlesSpawned = true;
				mTimeToSpawnParticles = Rand::randInt(500, 550);
			}
		}

		else
			mThousandParticlesSpawned = false;
	}

	float cElapsed = (float)getElapsedSeconds();
	vec3 cUpDir, cRightDir;
	mCamera.getBillboardVectors(&cRightDir, &cUpDir);
	for (auto pit = mPointsSnow.begin(); pit != mPointsSnow.end();)
	{
		pit->step(cElapsed, mPerlin, cRightDir);
		if (pit->Age <= 0)
			pit = mPointsSnow.erase(pit);
		else
			++pit;
	}

	mSnowDataInstance->bufferData(mPointsSnow.size()*sizeof(CloudParticle), mPointsSnow.data(), GL_DYNAMIC_DRAW);

}

void WinterParticlesButterflyApp::updateFBO()
{
	gl::ScopedViewport cFboView(0, 0, mFboRaw->getWidth(), mFboRaw->getHeight());

	mFboRaw->bindFramebuffer();
	gl::pushMatrices();
	gl::setMatrices(mCamera);
	gl::clear(ColorA::zero());
	drawParticles();
	drawPointCloud();
	gl::popMatrices();
	mFboRaw->unbindFramebuffer();

	//Blur U
	mFboBlurU->bindFramebuffer();
	gl::pushMatrices();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());
	mFboRaw->bindTexture(0);
	mShaderBlurU->uniform("u_SamplerFBO", 0);
	mShaderBlurU->bind();
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mFboRaw->unbindTexture(0);
	gl::popMatrices();
	mFboBlurU->unbindFramebuffer();

	//Blur V
	mFboBlurV->bindFramebuffer();
	gl::pushMatrices();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());
	mFboBlurU->bindTexture(0);
	mShaderBlurV->uniform("u_SamplerFBO", 0);
	mShaderBlurV->bind();
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mFboBlurU->unbindTexture(0);
	gl::popMatrices();
	mFboBlurV->unbindFramebuffer();
}

void WinterParticlesButterflyApp::draw()
{
	gl::clear(Color(0, 0, 0));
	gl::setMatrices(mMayaCam.getCamera());
	gl::enableDepthRead();
	gl::enableDepthWrite();

	drawSkyBox();
	drawPointCloud();
	drawParticles();

	gl::enableAdditiveBlending();
	gl::setMatricesWindow(getWindowWidth(), getWindowHeight());
	gl::draw(mFboBlurV->getColorTexture(), Rectf({ vec2(0), getWindowSize() }));
	gl::disableAlphaBlending();
}

void WinterParticlesButterflyApp::drawSkyBox()
{
	gl::disable(GL_DEPTH_TEST);
	mBatchSkyBox->getGlslProg()->uniform("u_ViewMatrix", mCamera.getViewMatrix());
	mTexSkyBox->bind();
	mBatchSkyBox->draw();
	mTexSkyBox->unbind();
	gl::enable(GL_DEPTH_TEST);
}

void WinterParticlesButterflyApp::drawPointCloud()
{
	mBatchCloud->getGlslProg()->uniform("ViewDirection", mMayaCam.getCamera().getViewDirection());
	mBatchCloud->getGlslProg()->uniform("LightPosition", vec3(0,100,0));
	
	gl::pushMatrices();
	gl::scale(vec3(1, -1, 1));
	mTexRgb->bind(0);
	mTexSkyBox->bind(1);
	mBatchCloud->drawInstanced(mPointsCloud.size());
	mTexRgb->unbind();
	mTexSkyBox->unbind();
	gl::popMatrices();
	
}

void WinterParticlesButterflyApp::drawParticles()
{
	mBatchSnow->getGlslProg()->uniform("u_SamplerRGB", 0);
	mBatchSnow->getGlslProg()->uniform("u_ViewDir", mMayaCam.getCamera().getViewDirection());
	mBatchSnow->getGlslProg()->uniform("u_LightPos", vec3(0, 100, 0));

	gl::pushMatrices();
	gl::scale(vec3(1, -1, 1));
	mPetalTex->bind(0);
	mBatchSnow->drawInstanced(mPointsSnow.size());
	mPetalTex->unbind();
	gl::popMatrices();
}

void WinterParticlesButterflyApp::cleanup()
{
	mCinderDS->stop();
}

CINDER_APP(WinterParticlesButterflyApp, RendererGl)
