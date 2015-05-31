#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/params/Params.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static int S_MAX_RINGS = 5;
static const vec2 S_BLUR_U = vec2(1.0, 0.0);
static const vec2 S_BLUR_V = vec2(0.0, 1.0);

class AR_DisplaceApp : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;

	void setupShaders();
	void setupFBO();
	void setupGUI();
	void drawFBO();

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
			Position = randVec3()*randFloat(0.01f, 0.1f);
			Position.y = 0.0f;
			Center = Position;
			
			OuterRadius = randFloat(0.25f, 0.4f);
			InnerRadius = OuterRadius - randFloat(0.0025f, 0.0075f);
			
			RingShape = gl::VboMesh::create(geom::Torus().radius(OuterRadius, InnerRadius).subdivisionsHeight(4).subdivisionsAxis(64));
			RingColor = ColorA(randFloat(0.25f, 1.0f), randFloat(0.1f,0.25f), randFloat(0.25f, 1.0f), 1.0f);

			Life = randInt(60, 240);
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
			float fadeTime = Life / 3.0f;

			if (Age > (Life - fadeTime))
				normAge = (Life - Age) / fadeTime;
			else if (Age >= 0 && Age <= fadeTime)
				normAge = Age / fadeTime;

			RingColor.a = normAge;
		}
	};

private:
	vector<Ringu>	mRings;
	CameraPersp		mCamera;

	gl::GlslProgRef	mRingShader,
					mBlurShader;

	gl::FboRef		mRawFbo,
					mBlurHFbo,
					mBlurVFbo;

	params::InterfaceGlRef mGUI;
	float					mParamBlurStrength,
							mParamBlurSizeU,
							mParamBlurSizeV;
	
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
}

void AR_DisplaceApp::update()
{
	if (mRings.size() < S_MAX_RINGS)
		mRings.push_back(Ringu());
	for (auto r = mRings.begin(); r != mRings.end();)
	{
		r->Step(getElapsedFrames());
		if (r->Age <= 0)
			r = mRings.erase(r);
		else
			++r;
	}

	drawFBO();
}

void AR_DisplaceApp::draw()
{
	gl::clear(Color::black());
	gl::color(Color::white());
	gl::setMatricesWindow(getWindowSize());

	gl::enableAlphaBlending();
	gl::draw(mRawFbo->getColorTexture());
	gl::disableAlphaBlending();

	gl::enableAdditiveBlending();
	gl::draw(mBlurVFbo->getColorTexture());
	gl::disableAlphaBlending();

	mGUI->draw();
}

void AR_DisplaceApp::setupGUI()
{
	mParamBlurSizeU = 1.0f;
	mParamBlurSizeV = 1.0f;
	mParamBlurStrength = 1.0f;

	mGUI = params::InterfaceGl::create("Params", ivec2(300, 200));
	mGUI->addParam<float>("paramBlurU", &mParamBlurSizeU).optionsStr("label = 'Blur Width'");
	mGUI->addParam<float>("paramBlurV", &mParamBlurSizeV).optionsStr("label = 'Blur Height'");
	mGUI->addParam<float>("paramBlurStr", &mParamBlurStrength).optionsStr("label = 'Blur Strength'");
}

void AR_DisplaceApp::setupShaders()
{
	mRingShader = gl::GlslProg::create(loadAsset("shaders/torus_vert.glsl"), loadAsset("shaders/torus_frag.glsl"));
	
	mBlurShader = gl::GlslProg::create(loadAsset("shaders/blur_vert.glsl"), loadAsset("shaders/blur_frag.glsl"));
	mBlurShader->uniform("uTextureSampler", 0);
}

void AR_DisplaceApp::setupFBO()
{
	mRawFbo = gl::Fbo::create(960, 540,gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA)));
	mBlurHFbo = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().dataType(GL_FLOAT).internalFormat(GL_RGBA32F)));
	mBlurVFbo = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().dataType(GL_FLOAT).internalFormat(GL_RGBA32F)));
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
		mRingShader->uniform("uColor", r.RingColor);
		gl::draw(r.RingShape);
		gl::popMatrices();
	}
	gl::disableAlphaBlending();
	mRawFbo->unbindFramebuffer();

	/////////////////////////////////////////////////////
	mBlurHFbo->bindFramebuffer();
	gl::enableAdditiveBlending();
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
	gl::enableAdditiveBlending();
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
}

CINDER_APP( AR_DisplaceApp, RendererGl(RendererGl::Options().msaa(16)) )
