#ifdef _DEBUG
#pragma comment(lib, "DSAPI.dbg.lib")
#else
#pragma comment(lib, "DSAPI.lib")
#endif
#include "cinder/app/App.h"
#include "cinder/app/AppBase.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"
#include "Cinder-DSAPI/src/CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

//DEBUG
static int S_NUM_HEDRONS = 100;
static int S_SPAWN_COUNT = 2;
static int S_SPAWN_TIME = 20;
class AR_GeoMaskApp : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;
	void cleanup() override;

	void setupCloud();
	void drawParticles();
	void drawCloud();
	void updateDS();

	struct HParticle
	{
		mat4	PModelMatrix;
		vec3	PPosition;
		vec3	PRotation;
		vec3	PSpeed;
		float	PSize;
		float	PAlpha;
		int		PAge;

		HParticle(){}
		HParticle(vec3 pPos) :PPosition(pPos)
		{
			PSize = randFloat(0.5f, 2.0f);
			PSpeed = vec3(0.0f, 0.0f, randFloat(0.025f, 0.15f));
			if (pPos.x < 0)
				PSpeed.x = randFloat(-0.005f, -0.01f);
			else
				PSpeed.x = randFloat(0.005f, 0.01f);
			PModelMatrix = mat4();
			PModelMatrix = glm::translate(PModelMatrix, PPosition);
			PRotation = vec3(randFloat(0.005f, 0.01f), randFloat(0.005f, 0.01f), randFloat(0.005f, 0.01f));
			PAlpha = 0.0f;
		}

		void Step(float pFrames)
		{
			PPosition.x += PSpeed.x; PPosition.z -= PSpeed.z;
			float nPos = 5.0f;
			if (PPosition.z >= 15.0f)
				nPos = 20.0f - PPosition.z;
			PAlpha = lmap<float>(nPos, 5.0f, 0.0f, 1.0f, 0.0f);

			PModelMatrix = mat4();
			PModelMatrix *= glm::translate(PModelMatrix, PPosition);

			auto rot = quat(PRotation*pFrames);
			PModelMatrix *= glm::toMat4(rot);
		}
	};

private:
	CinderDSRef			mDS;
	gl::Texture2dRef	mTexRgb;
	gl::VboRef			mCloudBuffer;
	vector<float>		mPoints;
	gl::GlslProgRef		mCloudShader;
	gl::BatchRef		mCloudBatch;

	gl::GlslProgRef		mShader;
	gl::GlslProgRef		mComposite;
	gl::VboMeshRef		mMesh;
	vector<HParticle>	mParticles;
	gl::VboRef			mInstances;
	gl::Texture2dRef	mTexBg;
	gl::BatchRef		mBatch;

	gl::FboRef			mParticleFbo;
	gl::FboRef			mCloudFbo;

	CameraPersp			mParticleCam;
	CameraPersp			mCloudCam;
	CameraUi			mCameraUi;

	int					mTimer;
	float				mPointSize;

	params::InterfaceGlRef	mGUI;
	float					mParamDist,
							mParamSinScale,
							mParamFrameScale,
							mParamRgbScale;
};

void AR_GeoMaskApp::setup()
{
	getWindow()->setSize(960, 540);
	setFrameRate(60.0f);
	mTexBg = gl::Texture::create(loadImage(loadAsset("textures/test_bg.png")));

	float pX = randFloat(-10.0f, 10.0f);
	float pY = randFloat(-10.0f, 10.0f);
	mParticles.push_back(HParticle(vec3(pX,pY,10.0f)));

	mShader = gl::GlslProg::create(loadAsset("shaders/particle_vert.glsl"), loadAsset("shaders/particle_frag.glsl"));
	mMesh = gl::VboMesh::create(geom::Icosahedron());
	mInstances = gl::Vbo::create(GL_ARRAY_BUFFER, mParticles, GL_DYNAMIC_DRAW);
	geom::BufferLayout attribs;
	attribs.append(geom::CUSTOM_0, 1, sizeof(HParticle), offsetof(HParticle, PSize), 1);
	attribs.append(geom::CUSTOM_1, 16, sizeof(HParticle), offsetof(HParticle, PModelMatrix), 1);
	attribs.append(geom::CUSTOM_2, 1, sizeof(HParticle), offsetof(HParticle, PAlpha), 1);

	mMesh->appendVbo(attribs, mInstances);
	mBatch = gl::Batch::create(mMesh, mShader, { { geom::CUSTOM_0, "iSize" },
													{ geom::CUSTOM_1, "iModelMatrix" },
													{ geom::CUSTOM_2, "iAlpha" } });

	mParticleCam.setPerspective(45.0f, getWindowAspectRatio(), 0.1f, 100.0f);
	mParticleCam.lookAt(vec3(0, 0, -19), vec3(0), vec3(0, 1, 0));
	mParticleCam.setPivotDistance(19);

	gl::enableDepthWrite();

	mTimer = getElapsedFrames();

	gl::Fbo::Format fboFormat;
	fboFormat.setColorTextureFormat(gl::Texture::Format().internalFormat(GL_RGBA));
	mParticleFbo = gl::Fbo::create(960, 540, fboFormat);
	mComposite = gl::GlslProg::create(loadAsset("shaders/passthru_vert.glsl"), loadAsset("shaders/comp_frag.glsl"));

	setupCloud();

	//GUI
	mGUI = params::InterfaceGl::create("Params", ivec2(200, 300));
	mGUI->addParam<float>("paramCamDist", &mParamDist).optionsStr("label='Camera Z'");
	mGUI->addParam<float>("paramCamMOve", &mParamSinScale).optionsStr("label='Camera X'");
	mGUI->addParam<float>("paramTimeScale", &mParamFrameScale).optionsStr("label='Frame Scale'");
	mGUI->addParam<float>("paramRgbScale", &mParamRgbScale).optionsStr("label='RGB Scale'");
	mParamDist = 750.0f;
	mParamSinScale = 250.0f;
	mParamFrameScale = 0.01f;
	mParamRgbScale = 1.0f;
}

void AR_GeoMaskApp::setupCloud()
{
	mPointSize = (float)getWindowWidth() / 480.0f;
	mTexRgb = gl::Texture2d::create(640, 480);
	mCloudShader = gl::GlslProg::create(loadAsset("shaders/cloud_vert.glsl"), loadAsset("shaders/cloud_frag.glsl"));
	//setup cloud vbo
	for (int i = 0; i < 480 * 360; ++i)
	{
		mPoints.push_back(0.0f);
		mPoints.push_back(0.0f);
		mPoints.push_back(0.0f);
		mPoints.push_back(0.0f);
		mPoints.push_back(0.0f);
	}

	mCloudBuffer = gl::Vbo::create(GL_ARRAY_BUFFER, mPoints.size()*sizeof(float) , mPoints.data(), GL_DYNAMIC_DRAW);
	geom::BufferLayout cloudAttribs;
	cloudAttribs.append(geom::POSITION, 3, sizeof(float) * 5, 0, 0);
	cloudAttribs.append(geom::TEX_COORD_0, 2, sizeof(float) * 5, sizeof(float) * 3, 0);
	
	auto vboMesh = gl::VboMesh::create(480 * 360, GL_POINTS, { { cloudAttribs, mCloudBuffer } });
	mCloudBatch = gl::Batch::create(vboMesh, mCloudShader);
	mCloudBatch->getGlslProg()->uniform("uTextureSampler", 0);
	mDS = CinderDSAPI::create();
	mDS->init();
	mDS->initDepth(FrameSize::DEPTHSD, 60);
	mDS->initRgb(FrameSize::RGBVGA, 60);
	mDS->start();

	mCloudCam.setPerspective(45.0f, getWindowAspectRatio(), 100.0f, 4000.0f);
	mCloudCam.lookAt(vec3(0), vec3(0, 0, 1), vec3(0, -1, 0));
	mCloudCam.setPivotDistance(500.0f);
	//mCameraUi = CameraUi(&mCloudCam, getWindow());

	gl::Fbo::Format fboFormat;
	fboFormat.setColorTextureFormat(gl::Texture::Format().internalFormat(GL_RGBA));
	mCloudFbo = gl::Fbo::create(960, 540, fboFormat);

}

void AR_GeoMaskApp::update()
{
	if (mParticles.size() < S_NUM_HEDRONS)
	{
		if (getElapsedFrames() - mTimer > S_SPAWN_TIME)
		{
			mTimer = getElapsedFrames();
			for (int j = 0; j < S_SPAWN_COUNT; ++j)
			{
				float pX = randFloat(-4.0f, 4.0f);
				float pY = randFloat(-8.0f, 8.0f);
				mParticles.push_back(HParticle(vec3(pX, pY, 20.0f)));
			}
		}
	}

	for (auto p = mParticles.begin(); p != mParticles.end();)
	{
		p->Step(static_cast<float>(getElapsedFrames()));
		if (p->PPosition.z <= -20.0f)
			p = mParticles.erase(p);
		else
			++p;
	}

	mInstances->bufferData(sizeof(HParticle)*mParticles.size(), mParticles.data(), GL_DYNAMIC_DRAW);
	/*float cameraX = -15.0f*math<float>::cos((M_PI / 2) + getElapsedFrames()*0.001f);
	float cameraZ = -15.0f*math<float>::sin((M_PI / 2)+getElapsedFrames()*0.001f);
	mCamera.lookAt(vec3(cameraX, 0.0f, cameraZ), vec3(0), vec3(0, 1, 0));*/

	updateDS();
}

void AR_GeoMaskApp::updateDS()
{
	float frame = getElapsedFrames();
	mDS->update();
	mTexRgb->update(mDS->getRgbFrame());
	Channel16u depthFrame = mDS->getDepthFrame();

	mPoints.clear();

	for (int dy = 0; dy < 360; dy++)
	{
		for (int dx = 0; dx < 480; dx++)
		{
			float dz = depthFrame.getValue(ivec2(dx, dy));
			if (dz > 100.0f && dz < 3000.0f)
			{
				vec3 worldPos = mDS->getDepthSpacePoint(vec3(dx, dy, dz));
				vec2 uvs = mDS->getColorCoordsFromDepthSpace(worldPos);
				mPoints.push_back(worldPos.x);
				mPoints.push_back(worldPos.y);
				mPoints.push_back(worldPos.z);
				mPoints.push_back(uvs.x);
				mPoints.push_back(uvs.y);
			}
		}
	}

	mCloudBuffer->bufferData(mPoints.size()*sizeof(float), mPoints.data(), GL_DYNAMIC_DRAW);
	vec3 eyePoint = vec3(mParamSinScale*math<float>::sin(frame*mParamFrameScale), 0.0f, 0.0f);
	mCloudCam.lookAt(eyePoint, vec3(0,0,mParamDist), vec3(0,-1,0));
}

void AR_GeoMaskApp::draw()
{
	drawParticles();
	drawCloud();
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	//gl::setMatrices(mCamera);
	
	gl::disableDepthRead();
	gl::setMatricesWindow(getWindowSize());

	gl::enableAlphaBlending();
	gl::color(0.1f, 0.1f, 0.1f);
	gl::draw(mCloudFbo->getColorTexture(), vec2(0));
	mComposite->bind();
	mComposite->uniform("uTextureSamplerRgb", 0);
	mComposite->uniform("uTextureSamplerMask", 1);
	mComposite->uniform("uRgbScale", mParamRgbScale);
	mCloudFbo->bindTexture(0);
	mParticleFbo->bindTexture(1);
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mParticleFbo->unbindTexture();
	mCloudFbo->unbindTexture();
	gl::disableAlphaBlending();

	mGUI->draw();
}

void AR_GeoMaskApp::drawParticles()
{
	mParticleFbo->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::enableDepthRead();
	gl::enableAlphaBlending();
	gl::setMatrices(mParticleCam);
	mBatch->getGlslProg()->uniform("uEyePos", mParticleCam.getEyePoint());
	mBatch->drawInstanced(S_NUM_HEDRONS);
	gl::disableAlphaBlending();
	mParticleFbo->unbindFramebuffer();
}

void AR_GeoMaskApp::drawCloud()
{
	mCloudFbo->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatrices(mCloudCam);
	gl::pointSize(mPointSize);
	mTexRgb->bind(0);
	mCloudBatch->draw();
	mTexRgb->unbind(0);
	mCloudFbo->unbindFramebuffer();
}
void AR_GeoMaskApp::cleanup()
{
	mDS->stop();
}

CINDER_APP( AR_GeoMaskApp, RendererGl(RendererGl::Options().msaa(16)))