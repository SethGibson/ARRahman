#include "cinder/Rand.h"
#include "Displace.h"
#include "../RahmanVisualsApp.h"

static int S_MAX_RINGS = 10;
Ringu::Ringu()
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
}

void Ringu::Step(float elapsedFrames)
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

Displace::Displace()
{

}

Displace::~Displace()
{

}

void Displace::Init(AppFrame *pParent)
{
	mParent = pParent;
	setupRings();
}

void Displace::Update(Channel16uRef pDepthChannel)
{
	mElapsedFrames = (float)mParent->getElapsedFrames();
	updateRings();
	updateDepth(pDepthChannel);
	updateFbo();
}

void Displace::Draw()
{
	gl::setMatrices(mCamera);
	gl::clear(Color::black());
	gl::disableDepthRead();
	gl::draw(mFboRaw->getColorTexture(), vec2(0));
	gl::enableAdditiveBlending();
	gl::draw(mFboBlurV->getColorTexture(), vec2(0));
	gl::disableAlphaBlending();
	gl::enableDepthRead();
}

void Displace::setupRings()
{
	mTexDepth = gl::Texture2d::create(480, 360);
	mFboRaw = gl::Fbo::create(960, 540, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().dataType(GL_FLOAT).internalFormat(GL_RGBA32F)));
	mFboBlurH = gl::Fbo::create(960, 540);
	mFboBlurV = gl::Fbo::create(960, 540);
	mFboDepth = gl::Fbo::create(960, 540);

	mShaderBlur = gl::GlslProg::create(mParent->loadAsset("shaders/_common/blur_vert.glsl"), mParent->loadAsset("shaders/_common/blur_frag.glsl"));
	mShaderBlur->uniform("uTextureSampler", 0);

	mShaderRing = gl::GlslProg::create(mParent->loadAsset("shaders/torus_vert.glsl"), mParent->loadAsset("shaders/torus_frag.glsl"));
	mRings.push_back(Ringu());
}

void Displace::updateRings()
{
	if (mRings.size() < S_MAX_RINGS)
	{
		mRings.push_back(Ringu());
	}
	for (auto r = mRings.begin(); r != mRings.end();)
	{
		r->Step(mElapsedFrames);
		if (r->Age <= 0)
			r = mRings.erase(r);
		else
			++r;
	}
}

void Displace::updateDepth(Channel16uRef pDepthChannel)
{
	Surface8u rgbSrf(480, 360, false);
	auto rgbIter = rgbSrf.getIter();

	while (rgbIter.line())
	{
		while (rgbIter.pixel())
		{
			rgbIter.r() = 0;
			rgbIter.g() = 0;
			rgbIter.b() = 0;
			float z = (float)pDepthChannel->getValue(ivec2(rgbIter.x(), rgbIter.y()));
			if (z > 0.0f&&z < 1500.0f)
			{
				uint8_t color = (uint8_t)lmap<float>(z, 100.0f, 1500.0f, 255.0f, 0.0f);
				rgbIter.r() = color;
				rgbIter.g() = color;
				rgbIter.b() = color;
			}
		}
	}

	mTexDepth->update(rgbSrf);

	mFboDepth->bindFramebuffer();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(Color::black());
	gl::color(Color::white());
	gl::draw(mTexDepth, Rectf({ vec2(0), getWindowSize() }));
	mFboDepth->unbindFramebuffer();
}
void Displace::updateFbo()
{
	mFboRaw->bindFramebuffer();
	gl::clear(ColorA::zero());
	gl::setMatrices(mCamera);
	gl::enableAlphaBlending();
	mShaderRing->bind();
	for (auto &r : mRings)
	{
		gl::pushMatrices();
		gl::translate(r.Position);
		gl::rotate(getElapsedFrames()*r.RotationSpeed, vec3(0, 1, 0));
		mShaderRing->uniform("uColor", r.RingColor);
		gl::draw(r.RingShape);
		gl::popMatrices();
	}
	gl::disableAlphaBlending();
	mFboRaw->unbindFramebuffer();

	/////////////////////////////////////////////////////
	mFboBlurH->bindFramebuffer();
	gl::enableAlphaBlending();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());

	mShaderBlur->bind();
	mShaderBlur->uniform("uBlurAxis", vec2(1.0, 0.0));
	//mBlurShader->uniform("uBlurSize", mParamBlurSizeU);
	//mBlurShader->uniform("uBlurStrength", mParamBlurStrength);
	mFboRaw->bindTexture();
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mFboRaw->unbindTexture();

	gl::disableAlphaBlending();
	mFboBlurH->unbindFramebuffer();

	/////////////////////////////////////////////////////
	mFboBlurV->bindFramebuffer();
	gl::enableAlphaBlending();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());

	mShaderBlur->uniform("uBlurAxis", vec2(0.0, 1.0));
	//mShaderBlur->uniform("uBlurSize", mParamBlurSizeV);
	//mShaderBlur->uniform("uBlurStrength", mParamBlurStrength);
	mFboBlurH->bindTexture();
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	mFboBlurH->unbindTexture();

	gl::disableAlphaBlending();
	mFboBlurV->unbindFramebuffer();
}