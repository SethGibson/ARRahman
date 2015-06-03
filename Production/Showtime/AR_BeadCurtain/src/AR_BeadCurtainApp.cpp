#ifdef _DEBUG
#pragma comment(lib, "DSAPI.dbg.lib")
#else
#pragma comment(lib, "DSAPI.lib")
#endif
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"
#include "Cinder-DSAPI/src/CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

static ivec2 S_WINDOW_SIZE(960, 540);
static int S_ROW_STEP = 20;
static int S_COL_STEP = 30;

class AR_BeadCurtainApp : public App
{
public:
	struct Bead
	{
		vec3	Position;
		vec2	Position2d;
		float	Radius;
		bool	Active;
		int		Life;
		int		Age;
		Color	EmissiveColor;
		Color	ActiveColor;

		Bead(){}
		Bead(vec3 pPos, vec2 pIPos) :Position(pPos), Position2d(pIPos)
		{
			Life = randInt(30, 90);
			Age = Life;
			//EmissiveColor = Color(Color8u(255, 140, 64));
			EmissiveColor = Color(Color8u(64, 160, 192));
			ActiveColor = Color::black();
			Radius = randFloat(0.35f, 1.0f);
			Active = false;
		}

		void Step(float frames)
		{
			if (Active&&Age > 0)
			{
				Age--;
				ActiveColor = EmissiveColor*((float)Age/(float)Life);
			}
			else if (Active&&Age == 0)
			{
				Active = false;
				Life = randInt(30, 90);
				Age = Life;
				ActiveColor = Color::black();
			}
		}
	};
	void setup() override;
	void update() override;
	void draw() override;
	void cleanup() override;
	void keyDown(KeyEvent pEvent) override;

	void setupFBO();
	void setupGUI();
	void setupDS();
	
	void drawFBO();

private:
	gl::GlslProgRef			mShader;
	gl::VboMeshRef			mBallMesh;
	gl::VboRef				mPositions;
	gl::BatchRef			mCurtain;
	gl::TextureCubeMapRef	mEnvMap;

	gl::FboRef				mRawFbo,
							mHiPassFbo,
							mBlurUFbo,
							mBlurVFbo,
							mCompFbo;

	gl::GlslProgRef			mHiPassShader,
							mBlurShader,
							mScreenShader;

	ivec2					mWindowSize;
	CameraPersp				mCamera;
	CameraUi				mCameraUi;

	vec2					mScale;

	vector<Bead>			mBeads;
	vector<Bead>			mActiveBeads;

	params::InterfaceGlRef	mGUI;
	bool					mDrawGUI;
	float					mParamWhiteMax,
							mParamWhiteMid,
							mParamWhiteThresh,
							mParamBlurStrength,
							mParamBlurSizeU,
							mParamBlurSizeV,
							mParamCameraSpeed,
							mParamCameraScale;

	CinderDSRef				mDS;
	Channel16u				mDepthChan;
};

void AR_BeadCurtainApp::setup()
{
	getWindow()->setSize(S_WINDOW_SIZE);
	setupGUI();
	setupFBO();
	setupDS();

	//setup shader
	mEnvMap = gl::TextureCubeMap::create(loadImage(loadAsset("textures/nightskybox.png")));
	mShader = gl::GlslProg::create(loadAsset("shaders/glass_ball_vert.glsl"), loadAsset("shaders/glass_ball_frag.glsl"));

	//setup geo
	mScale = vec2(480.0f / S_WINDOW_SIZE.x, 360.0f/S_WINDOW_SIZE.y);

	for (int dy = 0; dy < S_WINDOW_SIZE.y; dy += S_ROW_STEP)
	{
		float py = lmap<float>(dy, 0.0f, static_cast<float>(S_WINDOW_SIZE.y-S_ROW_STEP), -1.0f, 1.0f);
		for (int dx = 0; dx < S_WINDOW_SIZE.x; dx += S_COL_STEP)
		{
			float px = lmap<float>(dx, 0.0f, static_cast<float>(S_WINDOW_SIZE.x-S_COL_STEP), -1.7778f, 1.7778f);
			mBeads.push_back(Bead( vec3(static_cast<float>(px), static_cast<float>(py), 0.0f),
										vec2(dx*mScale.x, dy*mScale.y )
										));
		}
	}

	mBallMesh = gl::VboMesh::create(geom::Sphere().radius(0.035f));
	geom::BufferLayout attribs;

	mPositions = gl::Vbo::create(GL_ARRAY_BUFFER, mBeads, GL_DYNAMIC_DRAW);
	attribs.append(geom::CUSTOM_0, 3, sizeof(Bead), offsetof(Bead,Position), 1);
	attribs.append(geom::CUSTOM_1, 1, sizeof(Bead), offsetof(Bead, Radius), 1);
	attribs.append(geom::CUSTOM_2, 3, sizeof(Bead), offsetof(Bead, ActiveColor), 1);

	mBallMesh->appendVbo(attribs, mPositions);
	mCurtain = gl::Batch::create(mBallMesh, mShader, { { geom::CUSTOM_0, "iPosition" }, { geom::CUSTOM_1, "iSize" }, { geom::CUSTOM_2, "iColor" } });
	mCurtain->getGlslProg()->uniform("uCubemapSampler", 0);

	mCamera.setPerspective(45.0f,getWindowAspectRatio(), 0.1f, 100.0f);
	mCamera.lookAt(vec3(0, 0, -2.25f), vec3(0), vec3(0, 1, 0));
	mCamera.setPivotDistance(2.25f);
	mCameraUi = CameraUi(&mCamera, getWindow());
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void AR_BeadCurtainApp::keyDown(KeyEvent pEvent)
{
	if (pEvent.getChar() == 'd')
		mDrawGUI = !mDrawGUI;
}

void AR_BeadCurtainApp::setupFBO()
{
	mRawFbo = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().dataType(GL_FLOAT).internalFormat(GL_RGBA32F)));
	mHiPassFbo = gl::Fbo::create(960, 540);
	mBlurUFbo = gl::Fbo::create(960, 540);
	mBlurVFbo = gl::Fbo::create(960, 540);
	mCompFbo = gl::Fbo::create(960, 540);

	mHiPassShader = gl::GlslProg::create(loadAsset("shaders/passthru_vert.glsl"), loadAsset("shaders/highpass_frag.glsl"));
	mHiPassShader->uniform("uTextureSampler", 0);

	mBlurShader = gl::GlslProg::create(loadAsset("shaders/blur_vert.glsl"), loadAsset("shaders/blur_frag.glsl"));
	mBlurShader->uniform("uTextureSampler", 0);

	mScreenShader = gl::GlslProg::create(loadAsset("shaders/passthru_vert.glsl"), loadAsset("shaders/comp_frag.glsl"));
	mScreenShader->uniform("uTextureRgbSampler", 0);
	mScreenShader->uniform("uTextureBloomSampler", 1);
}

void AR_BeadCurtainApp::setupGUI()
{
	mDrawGUI = false;
	mParamWhiteMax =	1.1f;
	mParamWhiteMid =	0.25f;
	mParamWhiteThresh = 0.35f;
	
	mParamBlurSizeU =	1.5f;
	mParamBlurSizeV =	3.0f;
	mParamBlurStrength = 2.5f;

	mParamCameraScale =	0.25f;
	mParamCameraSpeed = 0.005f;

	mGUI = params::InterfaceGl::create("Params", vec2(300, 200));
	mGUI->addParam<float>("paramWhiteMax", &mParamWhiteMax).optionsStr("label = 'Luminance'");
	mGUI->addParam<float>("paramWhiteMid", &mParamWhiteMid).optionsStr("label = 'Midtone'");
	mGUI->addParam<float>("paramWhiteThresh", &mParamWhiteThresh).optionsStr("label = 'Threshold'");
	mGUI->addParam<float>("paramBlurSizeU", &mParamBlurSizeU).optionsStr("label = 'Blur Width'");
	mGUI->addParam<float>("paramBlurSizeV", &mParamBlurSizeV).optionsStr("label = 'Blur Height'");
	mGUI->addParam<float>("paramBlurStrength", &mParamBlurStrength).optionsStr("label = 'Blur Strength'");
	mGUI->addParam<float>("paramBlurCameraSpeed", &mParamCameraSpeed).optionsStr("label = 'Camera Speed'");
	mGUI->addParam<float>("paramBlurCameraScale", &mParamCameraScale).optionsStr("label = 'Camera Scale'");
}

void AR_BeadCurtainApp::setupDS()
{
	mDS = CinderDSAPI::create();
	mDS->init();
	mDS->initDepth(FrameSize::DEPTHSD, 60);
	mDS->start();

	mDepthChan = Channel16u(480, 360);
}

void AR_BeadCurtainApp::update()
{
	float eyeX = mParamCameraScale*math<float>::sin(getElapsedFrames()*mParamCameraSpeed);
	float eyeY = mParamCameraScale*math<float>::cos(getElapsedFrames()*mParamCameraSpeed);
	mCamera.lookAt(vec3(eyeX, eyeY, -2.25f), vec3(0), vec3(0, 1, 0));
	
	mDS->update();
	mDepthChan = mDS->getDepthFrame();

	//DEBUG!!
	for (auto b = mBeads.begin(); b != mBeads.end();++b)
	{
		vec2 pos = b->Position2d;
		pos.y = 360 - pos.y;
		float v = (float)mDepthChan.getValue(pos);
		if (v>100&&v<1500)
		{
			if (!b->Active)
				b->Active = true;
		}
		b->Step(getElapsedFrames());
	}

	mPositions->bufferData(mBeads.size()*sizeof(Bead), mBeads.data(), GL_DYNAMIC_DRAW);

	drawFBO();
}

void AR_BeadCurtainApp::draw()
{
	gl::clear(Color::black());
	gl::color(ColorA::white());
	gl::setMatricesWindow(getWindowSize());
	
	mScreenShader->bind();
	mRawFbo->bindTexture(0);
	mBlurVFbo->bindTexture(1);
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mBlurVFbo->unbindTexture(1);
	mRawFbo->unbindTexture(0);
	
	if (mDrawGUI)
		mGUI->draw();
}

void AR_BeadCurtainApp::cleanup()
{
	mDS->stop();
}

void AR_BeadCurtainApp::drawFBO()
{
	float lightPosX = 10.0f*math<float>::cos(getElapsedFrames()*0.001f);
	float lightPosZ = 10.0f*math<float>::sin(getElapsedFrames()*0.001f);

	///////////////////////////////////////////////////////////////////////////////////////////
	mRawFbo->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatrices(mCamera);
	gl::enableAlphaBlending();

	mEnvMap->bind(0);
	mCurtain->getGlslProg()->uniform("uEyePos", mCamera.getEyePoint());
	mCurtain->getGlslProg()->uniform("uLightPos", vec3(lightPosX, 10, lightPosZ));
	mCurtain->getGlslProg()->uniform("uRefraction", 1.0f / 2.42f);
	mCurtain->drawInstanced((S_WINDOW_SIZE.x / S_COL_STEP)*(S_WINDOW_SIZE.y / S_ROW_STEP));
	mEnvMap->unbind();

	gl::disableAlphaBlending();
	mRawFbo->unbindFramebuffer();

	///////////////////////////////////////////////////////////////////////////////////////////
	mHiPassFbo->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatricesWindow(getWindowSize());
	
	mHiPassShader->bind();
	mHiPassShader->uniform("uWhiteMax", mParamWhiteMax);
	mHiPassShader->uniform("uWhiteMid", mParamWhiteMid);
	mHiPassShader->uniform("uWhiteThreshold", mParamWhiteThresh);
	mRawFbo->bindTexture(0);
	gl::drawSolidRect(Rectf({vec2(0), getWindowSize()}));
	mRawFbo->unbindTexture();

	mHiPassFbo->unbindFramebuffer();

	///////////////////////////////////////////////////////////////////////////////////////////
	mBlurUFbo->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatricesWindow(getWindowSize());

	mBlurShader->bind();
	mBlurShader->uniform("uBlurAxis", vec2(1.0, 0.0));
	mBlurShader->uniform("uBlurStrength", mParamBlurStrength);
	mBlurShader->uniform("uBlurSize", mParamBlurSizeU);
	mHiPassFbo->bindTexture(0);
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mHiPassFbo->unbindTexture(0);

	mBlurUFbo->unbindFramebuffer();

	///////////////////////////////////////////////////////////////////////////////////////////
	mBlurVFbo->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatricesWindow(getWindowSize());

	mBlurShader->bind();
	mBlurShader->uniform("uBlurAxis", vec2(0.0, 1.0));
	mBlurShader->uniform("uBlurStrength", mParamBlurStrength);
	mBlurShader->uniform("uBlurSize", mParamBlurSizeV);
	mBlurUFbo->bindTexture(0);
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mBlurUFbo->unbindTexture(0);
	
	mBlurVFbo->unbindFramebuffer();
}

CINDER_APP( AR_BeadCurtainApp, RendererGl(RendererGl::Options().msaa(16)) )
