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
#include "cinder/Camera.h"
#include "cinder/GeomIo.h"
#include "cinder/CameraUi.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace ci::params;
using namespace std;
using namespace CinderDS;

static float S_MIN_Z = 100.0f;
static float S_MAX_Z = 1000.0f;
static float S_POINT_SIZE = 24.0f;
static int		S_SPAWN_COUNT = 1;
static int		S_MAX_COUNT = 10000;

class RandomSpawn : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void cleanup() override;

private:
	void setupDS();
	void setupScene();
	void setupParticles();
	void setupFBO();

	void updateParticles();
	void updateFBO();

	void drawParticles();

	//DSAPI
	CinderDSRef			mCinderDS;
	Channel16u			mChanDepth;
	ivec2				mDepthDims;

	struct CloudParticle
	{
		mat4 PModelMatrix;
		vec3 PPosition;
		vec3 PRotation;
		float PSize;
		float PSize_0;
		ColorA PColor;
		ColorA PColor_0;
		int PAge;
		int PLife;

		CloudParticle(vec3 pPos, float pSize, int pLife, ColorA pColor) :
			PPosition(pPos),
			PSize(pSize), PSize_0(pSize),
			PAge(pLife),PLife(pLife),
			PColor_0(pColor)

		{
			PColor = PColor_0;
			PModelMatrix = mat4();
			PModelMatrix *= glm::translate(PPosition);
			PRotation = vec3(randFloat(0.005f, 0.025f), randFloat(0.005f, 0.025f), randFloat(0.005f, 0.025f));
		}

		void step(Perlin &pPerlin)
		{
			float cNormAge = static_cast<float>(PAge) / static_cast<float>(PLife);

			auto rot = quat(PRotation);
			PModelMatrix *= toMat4(rot);

			PAge -= 1;
			PColor = lerp<ColorA>(ColorA(1,1,1,0), PColor_0, cNormAge);
			PSize = PSize_0 * cNormAge;
		}
	};

	//Particles
	vector<CloudParticle>	mPointsParticles;
	gl::VboRef				mDataInstance_P;
	geom::BufferLayout		mAttribsInstance_P;
	gl::VboMeshRef			mMesh_P;
	gl::GlslProgRef			mShader_P;
	gl::BatchRef			mBatch_P;
	Perlin					mPerlin;

	CameraPersp				mCamera;
	CameraUi				mMayaCam;
};

void RandomSpawn::setup()
{
	setupDS();
	setupScene();

	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::enableAlphaBlending();
}

void RandomSpawn::setupDS()
{
	mCinderDS = CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHQVGA, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);
	mCinderDS->start();

	mDepthDims = mCinderDS->getDepthSize();
	mChanDepth = Channel16u(mDepthDims.x, mDepthDims.y);
}

void RandomSpawn::setupScene()
{
	getWindow()->setSize(960, 540);
	setFrameRate(60);

	//Camera
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 100.0f, 4000.0f);
	mCamera.lookAt(vec3(0), vec3(0, 0, 1), vec3(0, -1, 0));
	mCamera.setPivotDistance(500.0f);
	mMayaCam = CameraUi(&mCamera, getWindow());

	setupParticles();
}

void RandomSpawn::setupParticles()
{
	mPointsParticles.clear();
	mDataInstance_P = gl::Vbo::create(GL_ARRAY_BUFFER, mPointsParticles, GL_DYNAMIC_DRAW);
	mAttribsInstance_P.append(geom::CUSTOM_0, 16, sizeof(CloudParticle), offsetof(CloudParticle, PModelMatrix), 1);
	mAttribsInstance_P.append(geom::CUSTOM_1, 1, sizeof(CloudParticle), offsetof(CloudParticle, PSize), 1);
	mAttribsInstance_P.append(geom::CUSTOM_2, 4, sizeof(CloudParticle), offsetof(CloudParticle, PColor), 1);

	mMesh_P = gl::VboMesh::create(geom::Icosahedron());
	mMesh_P->appendVbo(mAttribsInstance_P, mDataInstance_P);

	mShader_P = gl::GlslProg::create(loadAsset("shaders/particle.vert"), loadAsset("shaders/particle.frag"));
	mBatch_P = gl::Batch::create(mMesh_P, mShader_P, { { geom::CUSTOM_0, "iModelMatrix" }, { geom::CUSTOM_1, "iSize" }, { geom::CUSTOM_2, "iColor" } });

	mPerlin = Perlin();
}

void RandomSpawn::setupFBO()
{
}

void RandomSpawn::update()
{
	updateParticles();
}

void RandomSpawn::updateParticles()
{
	mCinderDS->update();
	mChanDepth = mCinderDS->getDepthFrame();
	//Channel16u::Iter cIter = mChanDepth.getIter();

	for (int sp = 0; sp < S_SPAWN_COUNT; ++sp)
	{
		if (mPointsParticles.size() < S_MAX_COUNT)
		{
			int cX = randInt(0, mDepthDims.x);
			int cY = randInt(0, mDepthDims.y);

			float cZ = (float)mChanDepth.getValue(ivec2(cX, cY));
			if (cZ > S_MIN_Z&&cZ < S_MAX_Z)
			{
				vec3 cWorld = mCinderDS->getDepthSpacePoint(static_cast<float>(mDepthDims.x-cX),
					static_cast<float>(cY),
					static_cast<float>(cZ));

				// pos, acc size life
				vec3 cAcc(randFloat(-5.f, 5.f), randFloat(0.5f, 3.5f), randFloat(-5.f, 5.f));
				float cSize = randFloat(S_POINT_SIZE*0.25f, S_POINT_SIZE);
				int cLife = randInt(60, 120);
				ColorA cColor = ColorA(randFloat(), 0.1f, randFloat(), 1.0f);

				mPointsParticles.push_back(CloudParticle(cWorld, cSize, cLife, cColor));
			}
		}
	}

	//now update particles
	for (auto pit = mPointsParticles.begin(); pit != mPointsParticles.end();)
	{
		if (pit->PAge == 0)
			pit = mPointsParticles.erase(pit);
		else
		{
			pit->step(mPerlin);
			++pit;
		}
	}

	mDataInstance_P->bufferData(mPointsParticles.size()*sizeof(CloudParticle), mPointsParticles.data(), GL_DYNAMIC_DRAW);
}

void RandomSpawn::updateFBO()
{

}

void RandomSpawn::draw()
{
	gl::clear(Color(0, 0, 0));
	gl::setMatrices(mCamera);
	drawParticles();


}

void RandomSpawn::drawParticles()
{
	mBatch_P->getGlslProg()->uniform("ViewDirection", mCamera.getViewDirection());
	mBatch_P->getGlslProg()->uniform("LightPosition", vec3(0, 10, 0));
	mBatch_P->getGlslProg()->uniform("SpecPow", 16.0f);
	mBatch_P->getGlslProg()->uniform("SpecStr", 1.0f);

	mBatch_P->drawInstanced(mPointsParticles.size());
}

void RandomSpawn::cleanup()
{
	mCinderDS->stop();
}

CINDER_APP(RandomSpawn, RendererGl)
