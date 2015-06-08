#include "cinder/app/App.h"
#include "cinder/Rand.h"
#include "BeadCurtain.h"
#include "../RahmanVisualsApp.h"

#pragma region Bead
Bead::Bead()
{

}

Bead::Bead(vec3 pPos, vec2 pIPos) :Position(pPos), Position2d(pIPos)
{
	Life = randInt(30, 90);
	Age = Life;
	EmissiveColor = Color(Color8u(255, 140, 64));
	//EmissiveColor = Color(Color8u(64, 160, 192));
	ActiveColor = Color::black();
	Radius = randFloat(0.35f, 1.0f);
	Active = false;
}

void Bead::Step(float frames)
{
	if (Active&&Age > 0)
	{
		Age--;
		ActiveColor = EmissiveColor*((float)Age / (float)Life);
	}
	else if (Active&&Age == 0)
	{
		Active = false;
		Life = randInt(30, 90);
		Age = Life;
		ActiveColor = Color::black();
	}
}
#pragma endregion

#pragma region Bead Curtain
static ivec2 S_WINDOW_SIZE(960, 540);
static int S_ROW_STEP = 30;
static int S_COL_STEP = 20;

BeadCurtain::BeadCurtain(){}
BeadCurtain::~BeadCurtain(){}

void BeadCurtain::Init(AppFrame *pParent)
{
	mParent = pParent;
	setupBeads();
	setupFbo();
}

void BeadCurtain::Update(Channel16uRef pDepthChannel)
{
	mElapsedFrames = (float)mParent->getElapsedFrames();
	updateBeads(pDepthChannel);
	updateFbo();
}

void BeadCurtain::Draw()
{
	gl::clear(Color::black());
	gl::setMatrices(mCamera);
	mCurtain->draw();

	gl::setMatricesWindowPersp(mParent->getWindowSize());
	gl::enableAdditiveBlending();
	gl::draw(mFboBlurV->getColorTexture(), vec2(0));
	gl::disableAlphaBlending();
}

void BeadCurtain::setupBeads()
{
	mEnvMap = gl::TextureCubeMap::create(loadImage(mParent->loadAsset("textures/beadcurtain/skybox.png")));
	mShaderBead = gl::GlslProg::create(mParent->loadAsset("shaders/beadcurtain/glass_ball_vert.glsl"), mParent->loadAsset("shaders/beadcurtain/glass_ball_frag.glsl"));

	//setup geo
	vec2 scale = vec2(480.0f / S_WINDOW_SIZE.x, 360.0f / S_WINDOW_SIZE.y);

	for (int dy = 0; dy < S_WINDOW_SIZE.y; dy += S_ROW_STEP)
	{
		float py = lmap<float>(dy, 0.0f, static_cast<float>(S_WINDOW_SIZE.y - S_ROW_STEP), -1.0f, 1.0f);
		for (int dx = 0; dx < S_WINDOW_SIZE.x; dx += S_COL_STEP)
		{
			float px = lmap<float>(dx, 0.0f, static_cast<float>(S_WINDOW_SIZE.x - S_COL_STEP), -1.7778f, 1.7778f);
			mBeads.push_back(Bead(vec3(static_cast<float>(px), static_cast<float>(py), 0.0f),
				vec2(dx*scale.x, dy*scale.y)
				));
		}
	}

	mBeadMesh = gl::VboMesh::create(geom::Sphere().radius(0.035f));
	geom::BufferLayout attribs;

	mPositions = gl::Vbo::create(GL_ARRAY_BUFFER, mBeads, GL_DYNAMIC_DRAW);
	attribs.append(geom::CUSTOM_0, 3, sizeof(Bead), offsetof(Bead, Position), 1);
	attribs.append(geom::CUSTOM_1, 1, sizeof(Bead), offsetof(Bead, Radius), 1);
	attribs.append(geom::CUSTOM_2, 3, sizeof(Bead), offsetof(Bead, ActiveColor), 1);

	mBeadMesh->appendVbo(attribs, mPositions);
	mCurtain = gl::Batch::create(mBeadMesh, mShaderBead, { { geom::CUSTOM_0, "iPosition" }, { geom::CUSTOM_1, "iSize" }, { geom::CUSTOM_2, "iColor" } });
	mCurtain->getGlslProg()->uniform("uCubemapSampler", 0);
}

void BeadCurtain::setupFbo()
{
	mFboRaw = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().dataType(GL_FLOAT).internalFormat(GL_RGBA32F)));
	mFboHiPass = gl::Fbo::create(960, 540);
	mFboBlurU = gl::Fbo::create(960, 540);
	mFboBlurV = gl::Fbo::create(960, 540);

	mShaderHiPass = gl::GlslProg::create(loadAsset("shaders/passthru_vert.glsl"), loadAsset("shaders/highpass_frag.glsl"));
	mShaderHiPass->uniform("uTextureSampler", 0);

	mShaderBlur = gl::GlslProg::create(loadAsset("shaders/blur_vert.glsl"), loadAsset("shaders/blur_frag.glsl"));
	mShaderBlur->uniform("uTextureSampler", 0);
}

void BeadCurtain::updateBeads(Channel16uRef pDepthChannel)
{
	for (auto b = mBeads.begin(); b != mBeads.end(); ++b)
	{
		vec2 pos = b->Position2d;
		pos.y = 360 - pos.y;
		float v = (float)pDepthChannel->getValue(pos);
		if (v>100 && v<1500)
		{
			if (!b->Active)
				b->Active = true;
		}
		b->Step(mElapsedFrames);
	}

	mPositions->bufferData(mBeads.size()*sizeof(Bead), mBeads.data(), GL_DYNAMIC_DRAW);
}

void BeadCurtain::updateFbo()
{
	float lightPosX = 10.0f*math<float>::cos(mElapsedFrames*0.001f);
	float lightPosZ = 10.0f*math<float>::sin(mElapsedFrames*0.001f);

	///////////////////////////////////////////////////////////////////////////////////////////
	mFboRaw->bindFramebuffer();
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
	mFboRaw->unbindFramebuffer();

	///////////////////////////////////////////////////////////////////////////////////////////
	mFboHiPass->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatricesWindow(getWindowSize());

	mShaderHiPass->bind();
	//mShaderHiPass->uniform("uWhiteMax", mParamWhiteMax);
	//mHiPassShader->uniform("uWhiteMid", mParamWhiteMid);
	//mHiPassShader->uniform("uWhiteThreshold", mParamWhiteThresh);
	mFboRaw->bindTexture(0);
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mFboRaw->unbindTexture();
	mFboHiPass->unbindFramebuffer();

	///////////////////////////////////////////////////////////////////////////////////////////
	mFboBlurU->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatricesWindow(getWindowSize());

	mShaderBlur->bind();
	//mBlurShader->uniform("uBlurAxis", vec2(1.0, 0.0));
	//mBlurShader->uniform("uBlurStrength", mParamBlurStrength);
	//mBlurShader->uniform("uBlurSize", mParamBlurSizeU);
	mFboHiPass->bindTexture(0);
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mFboHiPass->unbindTexture(0);

	mFboBlurU->unbindFramebuffer();

	///////////////////////////////////////////////////////////////////////////////////////////
	mFboBlurV->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatricesWindow(getWindowSize());

	mShaderBlur->bind();
	mShaderBlur->uniform("uBlurAxis", vec2(0.0, 1.0));
	//mBlurShader->uniform("uBlurStrength", mParamBlurStrength);
	//mBlurShader->uniform("uBlurSize", mParamBlurSizeV);
	//mBlurUFbo->bindTexture(0);
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mFboBlurU->unbindTexture(0);

	mFboBlurV->unbindFramebuffer();
}
#pragma endregion


