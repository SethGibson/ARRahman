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
#include "cinder/CameraUi.h"
#include "cinder/Rand.h"
#include "CiDSAPI.h"
#include "ParticleController.h"
#include "Particle.h"
#include "cinder/Perlin.h"
#include "cinder/ObjLoader.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class WinterParticlesButterflyApp : public App
{
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
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

	void drawPointCloud();
	void drawSkyBox();

	//DSAPI
	CinderDSRef			mCinderDS;
	gl::Texture2dRef	mTexRgb;

	

	//Particle controller
	ParticleController mParticleController;
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

	

	//model importing
	//ObjLoader loader;

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
	geom::Plane			mSnowMeshInstance;
	gl::VboMeshRef		mMeshSnow;
	gl::GlslProgRef		mShaderSnow;
	gl::BatchRef		mBatchSnow;

	ci::TriMeshRef      mTriMeshSnow;

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
	getWindow()->setSize(1280, 720);
	setFrameRate(60);

	//Camera
	//vec3 cEyePos = vec3(0, 5, 5);
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 1, 5000);
	mCamera.lookAt(vec3(0), vec3(0, 0, 1), vec3(0, 1, 0));
	mCamera.setPivotDistance(40);
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

	mMeshInstance = geom::Sphere().radius(2.0f);
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

	mSnowDataInstance = gl::Vbo::create(GL_ARRAY_BUFFER, mPointsSnow, GL_DYNAMIC_DRAW);
	mSnowAttribsInstance.append(geom::CUSTOM_4, 16, sizeof(Particle), offsetof(Particle, mTransform), 1);

	
	//mSnowMeshInstance = geom::Plane().size(vec2(1)).axes(vec3(0, 1, 0), vec3(1, 0, 0));

	ObjLoader loader(loadAsset("rosepetal.obj"));

	mTriMeshSnow = ci::TriMesh::create(loader);
	mMeshSnow = gl::VboMesh::create(*mTriMeshSnow);

	if (!loader.getAvailableAttribs().count(geom::NORMAL))
		mTriMeshSnow->recalculateNormals();

	mMeshSnow->appendVbo(mSnowAttribsInstance, mSnowDataInstance);

	mShaderSnow = gl::GlslProg::create(loadAsset("snow.vert"), loadAsset("snow.frag"));
	mBatchSnow = gl::Batch::create(*mTriMeshSnow, mShaderSnow, { { geom::CUSTOM_4, "iModelMatrix" }});
	
	mPetalTex = gl::Texture2d::create(loadImage(loadAsset("rosepetal.png")));
	


	//Skybox
	mTexSkyBox = gl::TextureCubeMap::create(loadImage(loadAsset("ph_cubemap.png")), gl::TextureCubeMap::Format().mipmap().internalFormat(GL_RGBA8));
	mShaderSkyBox = gl::GlslProg::create(loadAsset("skybox.vert"), loadAsset("skybox.frag"));
	mBatchSkyBox = gl::Batch::create(geom::Cube(), mShaderSkyBox);
	mBatchSkyBox->getGlslProg()->uniform("mTexCube", 0);
}

void WinterParticlesButterflyApp::setupFBO()
{
}

void WinterParticlesButterflyApp::mouseDown(MouseEvent event)
{
	mMayaCam.mouseDown(event.getPos());
}

void WinterParticlesButterflyApp::mouseDrag(MouseEvent event)
{
	mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), false, event.isRightDown());
}

void WinterParticlesButterflyApp::update()
{

	updatePointCloud();

	mTimeToSpawnParticles--;

	if (mTimeToSpawnParticles <= 0)
	{
		if (!mThousandParticlesSpawned)
		{
			mXIterator = mXIterator + 2;
			mZIterator = mZIterator + 2;

			// Set positions for where the particles need to be instanciated
			
			//Change the number in the for loop to generate more particles at a position per for loop iteration. Higher the number denser the particles.
			int noOfParticlesPerSpawn = 3;  
			int TotalNumberOfParticles = 150; // set total number of particles spawned here

			for (int i = 0; i < noOfParticlesPerSpawn; i++)
			{
				//mPointsSnow.push_back(Particle(vec3((float)mXIterator, (float)Rand::randInt(-300, 100), (float)(Rand::randInt(0, 600))), Rand::randVec3f()));
				mPointsSnow.push_back(Particle(vec3(Rand::randFloat(-50.0f, 50.0f), Rand::randFloat(70.0f, 30.0f), 100), Rand::randVec3f()));
			}

			mXDecrementer = mXDecrementer - 2;
			mZDecrementer = mZDecrementer - 2;

			for (int i = 0; i < noOfParticlesPerSpawn; i++)
			{
				//mPointsSnow.push_back(Particle(vec3((float)mXDecrementer, (float)Rand::randInt(-300, 100), (float)(Rand::randInt(0, 600))), Rand::randVec3f()));
				mPointsSnow.push_back(Particle(vec3(Rand::randFloat(-50.0f, 50.0f), Rand::randFloat(70.0f, 30.0f), 100), Rand::randVec3f()));
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



	//update particle locations and behaviors
	for (vector<Particle>::iterator p = mPointsSnow.begin(); p != mPointsSnow.end();){
		if (p->mIsDead){
			p = mPointsSnow.erase(p);
		}
		else {

			p->mTransform = glm::mat4();

			
			float nX = p->mLoc.x * 0.005f;
			float nY = p->mLoc.y * 0.005f;
			float nZ = app::getElapsedSeconds() * 0.1f;
			float noise = mPerlin.fBm(nX, nY, nZ) * 0.7;

			float elapsedFrames = (float)getElapsedFrames();

			//update location and add perlin noise
			p->mLoc += p->mDir*p->mVelocity + noise;

			//rotate here. 
			p->mTransform = glm::translate(p->mTransform, p->mLoc);
			p->mTransform = glm::rotate(p->mTransform, p->mRotSpeed *elapsedFrames* 0.1f, vec3(0.5,0.5,0.5));  // set axis here
			console() << p->mTransform << std::endl;

			//age particles
			p->mAge++;
			if (p->mAge > p->mLifespan) p->mIsDead = true;
			++p;
		}
	}
	
	mSnowDataInstance->bufferData(mPointsSnow.size()*sizeof(Particle), mPointsSnow.data(), GL_DYNAMIC_DRAW);

	ObjLoader loader(loadAsset("rosepetal.obj"));
	mTriMeshSnow = ci::TriMesh::create(loader);
	mMeshSnow = gl::VboMesh::create(*mTriMeshSnow);
	mMeshSnow->appendVbo(mSnowAttribsInstance, mSnowDataInstance);
	mBatchSnow->replaceVboMesh(mMeshSnow);



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
			if (cIter.x() % 8 == 0 && cIter.y() % 8 == 0)
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
	}

	mMeshCloud = gl::VboMesh::create(mMeshInstance);
	mDataInstance->bufferData(mPointsCloud.size()*sizeof(CloudPoint), mPointsCloud.data(), GL_DYNAMIC_DRAW);
	mMeshCloud->appendVbo(mAttribsInstance, mDataInstance);
	mBatchCloud->replaceVboMesh(mMeshCloud);

}

void WinterParticlesButterflyApp::updateFBO()
{

}

void WinterParticlesButterflyApp::draw()
{

	vec3 cCameraRight, cCameraUp;

	gl::clear(Color(0, 0, 0));
	gl::setMatrices(mMayaCam.getCamera());
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::enableAlphaBlending();
	mParticleController.draw();
	drawSkyBox();
	drawPointCloud();

	//draw particles
	mMayaCam.getCamera().getBillboardVectors(&cCameraRight, &cCameraUp);

	mBatchSnow->getGlslProg()->uniform("u_CameraUp", cCameraUp);
	mBatchSnow->getGlslProg()->uniform("u_CameraRight", cCameraRight);
	mBatchSnow->getGlslProg()->uniform("u_SamplerRGB", 0);

	gl::pushMatrices();
	gl::scale(vec3(1, -1, 1));
	mPetalTex->bind(0);
	mBatchSnow->drawInstanced(mPointsSnow.size());
	mPetalTex->unbind();
	gl::popMatrices();
	gl::disableAlphaBlending();

	
}

void WinterParticlesButterflyApp::drawSkyBox()
{
	gl::pushMatrices();
	gl::scale(2000, 2000, 2000);
	mTexSkyBox->bind();
	mBatchSkyBox->draw();
	mTexSkyBox->unbind();
	gl::popMatrices();

}

void WinterParticlesButterflyApp::drawPointCloud()
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

void WinterParticlesButterflyApp::cleanup()
{
	mCinderDS->stop();
}

CINDER_APP(WinterParticlesButterflyApp, RendererGl)
