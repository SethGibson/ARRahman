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
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"
#include "Cinder-DSAPI/src/CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

static int S_MAX_RINGS = 10;
static const vec2 S_BLUR_U = vec2(1.0, 0.0);
static const vec2 S_BLUR_V = vec2(0.0, 1.0);

class AR_DisplaceApp : public App
{
public:
	struct Ringu
	{
		vec3				Position, Center;
		float				InnerRadius,
							OuterRadius,
							RotationSpeed;

		int					Age, Life;
		ColorA				RingColor;

		gl::VboMeshRef		RingShape;

		Ringu()
		{
			Position = randVec3()*randFloat(0.05f, 0.25f);
			Position.y = 0.0f;
			Center = Position;

			OuterRadius = randFloat(0.1f, 0.4f);
			InnerRadius = OuterRadius - randFloat(0.001f, 0.0075f);

			RingShape = gl::VboMesh::create(geom::Torus().radius(OuterRadius, InnerRadius).subdivisionsHeight(4).subdivisionsAxis(64));
			RingColor = ColorA(randFloat(0.25f, 1.0f), randFloat(0.1f, 0.25f), randFloat(0.25f, 1.0f), 1.0f);

			Life = randInt(120, 240);
			Age = Life;
			RotationSpeed = randFloat(0.005, 0.01f);
			if (Center.x<0)
				RotationSpeed = randFloat(-0.005, -0.01f);
		};

		void Step(float elapsedFrames)
		{
			Position.x = Center.x * math<float>::cos(elapsedFrames*RotationSpeed);
			Position.z = Center.z * math<float>::sin(elapsedFrames*RotationSpeed);

			Age--;
			float normAge = 1.0f;
			float fadeTime = Life / 2.0f;

			if (Age > (Life - fadeTime))
				normAge = (Life - Age) / fadeTime;
			else if (Age >= 0 && Age <= fadeTime)
				normAge = Age / fadeTime;

			RingColor.a = normAge;
		}
	};

	void setup() override;
	void keyDown(KeyEvent pEvent) override;
	void update() override;
	void draw() override;
	void cleanup() override;

	void setupShaders();
	void setupFBO();
	void setupGUI();
	void setupDS();

	void updateDS();

	void drawFBO();

private:
	vector<Ringu>	mRings;
	CameraPersp		mCamera;

	gl::GlslProgRef	mRingShader,
					mBlurShader,
					mCompShader;

	gl::FboRef		mRawFbo,
					mBlurHFbo,
					mBlurVFbo,
					mCompFbo;

	params::InterfaceGlRef mGUI;
	bool					mDrawGUI;
	float					mParamBlurStrength,
							mParamBlurSizeU,
							mParamBlurSizeV,
							mParamDisplaceAmt,
							mParamDepthMax,
							mParamErrorTerm;

	//Depth map
	CinderDSRef				mDS;
	gl::Texture2dRef		mTexDepth;
	gl::Texture2dRef		mTexRawDepth;

	gl::GlslProgRef			mDepthShader;
	gl::FboRef				mDepthFbo;

	int						mTimer;
	int						mSpawnTime;
};

void AR_DisplaceApp::setup()
{
	getWindow()->setSize(960, 540);
	setFrameRate(60.0f);
	
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 0.01f, 1.0f);
	mCamera.lookAt(vec3(0, -1, 0), vec3(0), vec3(0, 0, 1));

	mRings.push_back(Ringu());
	
	setupGUI();
	setupShaders();
	setupFBO();
	setupDS();

}

void AR_DisplaceApp::keyDown(KeyEvent pEvent)
{
	if (pEvent.getChar() == 'd')
		mDrawGUI = !mDrawGUI;
}

void AR_DisplaceApp::update()
{
	if (mRings.size() < S_MAX_RINGS)
	{
		mRings.push_back(Ringu());
	}
	for (auto r = mRings.begin(); r != mRings.end();)
	{
		r->Step(getElapsedFrames());
		if (r->Age <= 0)
			r = mRings.erase(r);
		else
			++r;
	}
	updateDS();
	drawFBO();
}

void AR_DisplaceApp::draw()
{
	gl::clear(Color::black());
	gl::color(Color::white());
	
	gl::setMatricesWindow(getWindowSize());
	gl::pushMatrices();
	gl::translate(vec3(960, 0, 0));
	gl::scale(vec3(-1, 1,1));

	gl::draw(mCompFbo->getColorTexture(), vec2(0));
	gl::popMatrices();

	if (mDrawGUI)
		mGUI->draw();
}

void AR_DisplaceApp::setupGUI()
{
	mDrawGUI = false;
	mParamBlurSizeU = 1.5f;
	mParamBlurSizeV = 3.0f;
	mParamBlurStrength = 3.f;
	mParamDisplaceAmt = 0.25f;
	mParamDepthMax = 1200.0f;
	mParamErrorTerm = 32768.0f;

	mGUI = params::InterfaceGl::create("Params", ivec2(300, 200));
	mGUI->addParam<float>("paramBlurU", &mParamBlurSizeU).optionsStr("label = 'Blur Width'");
	mGUI->addParam<float>("paramBlurV", &mParamBlurSizeV).optionsStr("label = 'Blur Height'");
	mGUI->addParam<float>("paramBlurStr", &mParamBlurStrength).optionsStr("label = 'Blur Strength'");
	mGUI->addParam<float>("paramDisplace", &mParamDisplaceAmt).optionsStr("label = 'Diplacement'");
	mGUI->addParam<float>("paramDepthMax", &mParamDepthMax).optionsStr("label = 'Max Depth'");
	mGUI->addParam<float>("paramErrorTerm", &mParamErrorTerm).optionsStr("label = 'Error Term'");
}

void AR_DisplaceApp::setupShaders()
{
	mRingShader = gl::GlslProg::create(loadAsset("shaders/torus_vert.glsl"), loadAsset("shaders/torus_frag.glsl"));
	
	mBlurShader = gl::GlslProg::create(loadAsset("shaders/blur_vert.glsl"), loadAsset("shaders/blur_frag.glsl"));
	mBlurShader->uniform("uTextureSampler", 0);

	mCompShader = gl::GlslProg::create(loadAsset("shaders/passthru_vert.glsl"), loadAsset("shaders/comp_frag.glsl"));
	mCompShader->uniform("uTextureRgbSampler",0);
	mCompShader->uniform("uTextureBloomSampler", 1);
	mCompShader->uniform("uTextureDepthSampler", 2);

	mDepthShader = gl::GlslProg::create(loadAsset("shaders/passthru_vert.glsl"), loadAsset("shaders/depth_remap_frag.glsl"));
	mDepthShader->uniform("uDepthSampler", 0);

	mTexRawDepth = gl::Texture2d::create(480, 360, gl::Texture2d::Format().dataType(GL_HALF_FLOAT).internalFormat(GL_R16F));
}

void AR_DisplaceApp::setupFBO()
{
	mRawFbo = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().dataType(GL_FLOAT).internalFormat(GL_RGBA32F)));
	mBlurHFbo = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGB)));
	mBlurVFbo = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGB)));
	mCompFbo = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGB)));
	mDepthFbo = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGB)));
}

void AR_DisplaceApp::setupDS()
{
	mDS = CinderDSAPI::create();
	
	mDS->init();
	mDS->initDepth(FrameSize::DEPTHSD, 60);
	mTexDepth = gl::Texture2d::create(480, 360);

	mDS->start();



}

void AR_DisplaceApp::updateDS()
{
	mDS->update();
	
	Surface8u rgbSrf(480, 360,false);
	auto rgbIter = rgbSrf.getIter();
	auto depthChan = mDS->getDepthFrame().getData();
	mTexRawDepth->update(mDS->getDepthFrame());

	while (rgbIter.line())
	{
		while (rgbIter.pixel())
		{
			rgbIter.r() = 0;
			rgbIter.g() = 0;
			rgbIter.b() = 0;
			float z = (float)depthChan[rgbIter.y() * 480 + rgbIter.x()];
			if (z > 0.0f&&z < mParamDepthMax)
			{
				uint8_t color = (uint8_t)lmap<float>(z, 100.0f, mParamDepthMax, 255.0f, 0.0f);
				rgbIter.r() = color;
				rgbIter.g() = color;
				rgbIter.b() = color;
			}
		}
	}

	mTexDepth->update(rgbSrf);

	mDepthFbo->bindFramebuffer();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(Color::black());
	gl::color(Color::white());
	gl::draw(mTexDepth, Rectf({vec2(0), getWindowSize()}));
	mDepthFbo->unbindFramebuffer();
}

void AR_DisplaceApp::drawFBO()
{
	mRawFbo->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatrices(mCamera);
	gl::enableAlphaBlending();
	mRingShader->bind();
	for (auto r : mRings)
	{
		gl::pushMatrices();
		gl::translate(r.Position);
		gl::rotate(getElapsedFrames()*r.RotationSpeed, vec3(0, 1, 0));
		mRingShader->uniform("uColor", r.RingColor);
		gl::draw(r.RingShape);
		gl::popMatrices();
	}
	gl::disableAlphaBlending();
	mRawFbo->unbindFramebuffer();

	/////////////////////////////////////////////////////
	mBlurHFbo->bindFramebuffer();
	gl::enableAlphaBlending();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());

	mBlurShader->bind();
	mBlurShader->uniform("uBlurAxis", S_BLUR_U);
	mBlurShader->uniform("uBlurSize", mParamBlurSizeU);
	mBlurShader->uniform("uBlurStrength", mParamBlurStrength);
	mRawFbo->bindTexture();
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mRawFbo->unbindTexture();

	gl::disableAlphaBlending();
	mBlurHFbo->unbindFramebuffer();

	/////////////////////////////////////////////////////
	mBlurVFbo->bindFramebuffer();
	gl::enableAlphaBlending();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());

	mBlurShader->uniform("uBlurAxis", S_BLUR_V);
	mBlurShader->uniform("uBlurSize", mParamBlurSizeV);
	mBlurShader->uniform("uBlurStrength", mParamBlurStrength);
	mBlurHFbo->bindTexture();
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mBlurHFbo->unbindTexture();

	gl::disableAlphaBlending();
	mBlurVFbo->unbindFramebuffer();

	/////////////////////////////////////////////////////
	mCompFbo->bindFramebuffer();
	gl::enableAlphaBlending();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());

	mCompShader->bind();
	mCompShader->uniform("uDisplacementAmount", mParamDisplaceAmt);
	mRawFbo->bindTexture(0);
	mBlurVFbo->bindTexture(1);
	mDepthFbo->bindTexture(2);
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));

	mDepthFbo->unbindTexture();
	mBlurVFbo->unbindTexture();
	mRawFbo->unbindTexture();
	gl::disableAlphaBlending();
	mCompFbo->unbindFramebuffer();
	/////////////////////////////////////////////////////
	//mDepthFbo
}

void AR_DisplaceApp::cleanup()
{
	mDS->stop();
}

CINDER_APP( AR_DisplaceApp, RendererGl(RendererGl::Options().msaa(16)) )
