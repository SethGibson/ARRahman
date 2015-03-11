#ifdef _DEBUG
#pragma comment(lib, "DSAPI.dbg.lib")
#else
#pragma comment(lib, "DSAPI.lib")
#endif

#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/params/Params.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class LitCloudApp : public AppNative
{
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void update() override;
	void draw() override;

	void exit();

	struct CloudPoint
	{
		vec3 IPosition;
		vec4 IColor;
		CloudPoint(vec3 pPos, vec4 pColor) : IPosition(pPos), IColor(pColor){}
	};

private:
	void setupGUI();
	void setupDSAPI();
	void setupMeshes();
	void setupFBOs();

	void renderFBOs();

	//PointCloud
	geom::Sphere			mCloudCube;
	gl::BatchRef			mCloudBatch;
	geom::BufferLayout		mCloudAttribs;
	gl::VboRef				mCloudData;
	gl::VboMeshRef			mCloudMesh;
	gl::GlslProgRef			mCloudShader;
	
	//FSFX
	gl::FboRef				mCloudFbo;
	gl::Texture2dRef		mTexFSFX;
	gl::GlslProgRef			mFSFXShader;
	float					mDisplaceAmount,
							mScrollSpeed;

	//Skybox
	gl::BatchRef			mSkyBatch;
	gl::TextureCubeMapRef	mSkyTexCube;
	gl::GlslProgRef			mSkyShader;

	//Lighting
	Color					mLightColor;
	float					mAmbientScale,
							mSpecularScale,
							mSpecularPower,
							mEnvScale;

	CameraPersp				mCamera;
	MayaCamUI				mMayaCam;
	vector<CloudPoint>		mPositions;

	CinderDSRef				mCinderDS;
	ivec2					mDepthDims;

	// GUI
	float					mColorScale;
	params::InterfaceGlRef	mGUI;
};

void LitCloudApp::setup()
{
	getWindow()->setSize(1280, 720);
	setupGUI();
	setupDSAPI();
	setupMeshes();
	setupFBOs();

	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 0.1, 2000.0f);
	mCamera.lookAt(vec3(0, 0, -3), vec3(0), vec3(0, 1, 0));
	mCamera.setCenterOfInterestPoint(vec3(0, 0, 1));
	mMayaCam.setCurrentCam(mCamera);

}

void LitCloudApp::setupGUI()
{
	mColorScale = 1.5f;
	mAmbientScale = 0.8f;
	mSpecularScale = 1.0f;
	mSpecularPower = 16.0f;
	mEnvScale = 1.0f;
	mDisplaceAmount = 0.1f;
	mScrollSpeed = 0.1f;

	mLightColor = Color::white();

	mGUI = params::InterfaceGl::create("Params", vec2(300, 300));
	mGUI->addParam("RGB Scale", &mColorScale).min(0.1f).max(3.0f).step(0.1f);
	mGUI->addSeparator();
	mGUI->addParam("Light Color", &mLightColor);
	mGUI->addParam("Ambient Scale", &mAmbientScale).min(0.1f).max(1.0f).step(0.1f);
	mGUI->addParam("Specular Scale", &mSpecularScale).min(0.1f).max(3.0f).step(0.1f);
	mGUI->addParam("Specular Power", &mSpecularPower).min(4.0f).max(128.0f).step(1.0f);
	mGUI->addParam("Reflect Scale", &mEnvScale).min(0.1f).max(3.0f).step(0.1f);
	mGUI->addSeparator();
	mGUI->addParam("Scroll Speed", &mScrollSpeed).min(0.1f).max(1.0f).step(0.1f);
	mGUI->addParam("Noise Amount", &mDisplaceAmount).min(0.0f).max(1.0f).step(0.01f);
}

void LitCloudApp::setupDSAPI()
{
	mCinderDS = CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHSD, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);
	mDepthDims = mCinderDS->getDepthSize();
	mCinderDS->start();

	getSignalShutdown().connect(std::bind(&LitCloudApp::exit, this));
}

void LitCloudApp::setupMeshes()
{
	try
	{
		mCloudShader = gl::GlslProg::create(loadAsset("diff_spec.vert"), loadAsset("diff_spec.frag"));
		mSkyShader = gl::GlslProg::create(loadAsset("sky_box.vert"), loadAsset("sky_box.frag"));
	}
	catch (const gl::GlslProgExc &e)
	{
		console() << e.what() << endl;
	}
	mPositions.clear();

	for (int y = 0; y < mDepthDims.y; ++y)
	{
		for (int x = 0; x < mDepthDims.x; ++x)
		{
			float cx = lmap<float>(x, 0, mDepthDims.x, -2.666f, 2.666f);
			float cy = lmap<float>(y, 0, mDepthDims.y, -2.0f, 2.0f);
			mPositions.push_back(CloudPoint(vec3(cx, cy, 0), vec4(1)));
		}
	}

	mSkyTexCube = gl::TextureCubeMap::create(loadImage(loadAsset("env_map.jpg")), gl::TextureCubeMap::Format().mipmap().internalFormat(GL_RGBA8));
	mSkyBatch = gl::Batch::create(geom::Cube(), mSkyShader);
	mSkyBatch->getGlslProg()->uniform("uCubeMapTex", 0);
	
	mCloudCube = geom::Sphere().radius(0.0125f).subdivisions(8);
	mCloudMesh = gl::VboMesh::create(mCloudCube);

	mCloudData = gl::Vbo::create(GL_ARRAY_BUFFER, mPositions, GL_DYNAMIC_DRAW);
	mCloudAttribs.append(geom::CUSTOM_0, 3, sizeof(CloudPoint), offsetof(CloudPoint, IPosition), 1);
	mCloudAttribs.append(geom::CUSTOM_1, 4, sizeof(CloudPoint), offsetof(CloudPoint, IColor), 1);

	mCloudMesh->appendVbo(mCloudAttribs, mCloudData);
	mCloudBatch = gl::Batch::create(mCloudMesh, mCloudShader, { { geom::CUSTOM_0, "iPosition" }, { geom::CUSTOM_1, "iColor" } });
	mCloudBatch->getGlslProg()->uniform("uCubeMapTex", 0);
	mCloudBatch->getGlslProg()->uniform("ViewDirection", mMayaCam.getCamera().getViewDirection());
	mCloudBatch->getGlslProg()->uniform("LightColor", mLightColor);
	mCloudBatch->getGlslProg()->uniform("LightPosition", mMayaCam.getCamera().getEyePoint());
	mCloudBatch->getGlslProg()->uniform("SpecularScale", mSpecularScale);
	mCloudBatch->getGlslProg()->uniform("SpecularPower", mSpecularPower);
	mCloudBatch->getGlslProg()->uniform("AmbientScale", mAmbientScale);
	mCloudBatch->getGlslProg()->uniform("EnvScale", mEnvScale);
}

void LitCloudApp::setupFBOs()
{
	mCloudFbo = gl::Fbo::create(1280, 720, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA8)));
	mTexFSFX = gl::Texture2d::create(loadImage(loadAsset("water_tex.png")), gl::Texture2d::Format().wrap(GL_REPEAT));

	try
	{
		mFSFXShader = gl::GlslProg::create(loadAsset("passthru.vert"), loadAsset("fsfx_displace.frag"));
	}
	catch (const gl::GlslProgExc &e)
	{
		console() << e.what() << endl;
	}


}

void LitCloudApp::mouseDown(MouseEvent event)
{
	mMayaCam.mouseDown(event.getPos());
}

void LitCloudApp::mouseDrag(MouseEvent event)
{
	mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), false, event.isRightDown());
}

void LitCloudApp::update()
{
	mCinderDS->update();
	mPositions.clear();
	Channel16u cChanDepth = mCinderDS->getDepthFrame();
	Channel16u::Iter cIter = cChanDepth.getIter();

	while (cIter.line())
	{
		while (cIter.pixel())
		{
			if (cIter.x() % 2 == 0 && cIter.y() % 2 == 0)
			{
				float cVal = (float)cIter.v();
				if (cVal > 100 && cVal < 1000)
				{
					float cx = lmap<float>(cIter.x(), 0, mDepthDims.x, -8.f, 8.f);
					float cy = lmap<float>(cIter.y(), 0, mDepthDims.y, 6.f, -6.f);
					float cz = lmap<float>(cVal, 100.0f, 1000.0f, -8.f, 8.f);

					Color cColor = mCinderDS->getDepthSpaceColor(vec3(cIter.x(), cIter.y(), cVal));
					mPositions.push_back(CloudPoint(vec3(cx, cy, cz), vec4(cColor.r, cColor.g, cColor.b, 1)));
				}
			}
		}
	}

	mCloudData->bufferData(mPositions.size()*sizeof(vec3), mPositions.data(), GL_DYNAMIC_DRAW);
	mCloudMesh = gl::VboMesh::create(mCloudCube);
	mCloudMesh->appendVbo(mCloudAttribs, mCloudData);
	mCloudBatch->replaceVboMesh(mCloudMesh);

	renderFBOs();
}

void LitCloudApp::renderFBOs()
{
	gl::ScopedFramebuffer cFbo(mCloudFbo);
	gl::ScopedTextureBind cTex(mSkyTexCube);
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::enableAlphaBlending();
	gl::setMatrices(mMayaCam.getCamera());
	gl::clear(ColorA::zero());

	mCloudBatch->getGlslProg()->uniform("ViewDirection", mMayaCam.getCamera().getViewDirection());
	mCloudBatch->getGlslProg()->uniform("LightColor", mLightColor);
	mCloudBatch->getGlslProg()->uniform("LightPosition", mMayaCam.getCamera().getEyePoint());
	mCloudBatch->getGlslProg()->uniform("SpecularScale", mSpecularScale);
	mCloudBatch->getGlslProg()->uniform("SpecularPower", mSpecularPower);
	mCloudBatch->getGlslProg()->uniform("AmbientScale", mAmbientScale);
	mCloudBatch->getGlslProg()->uniform("EnvScale", mEnvScale);
	mCloudBatch->drawInstanced(mPositions.size());
	gl::disableDepthRead();
	gl::disableDepthWrite();
	gl::disableAlphaBlending();
}

void LitCloudApp::draw()
{
	gl::clear(Color(0.1f, 0.15f, 0.25f));
	gl::setMatrices(mMayaCam.getCamera());

	gl::enableDepthRead();
	gl::pushMatrices();
	gl::scale(500, 500, 500);
	mSkyTexCube->bind();
	mSkyBatch->draw();
	gl::popMatrices();
	mSkyTexCube->unbind();
	gl::disableDepthRead();

	gl::setMatricesWindow(getWindowSize());

	gl::enableAdditiveBlending();
	mCloudFbo->bindTexture(0);
	mTexFSFX->bind(1);
	mFSFXShader->bind();
	mFSFXShader->uniform("mTexColor", 0);
	mFSFXShader->uniform("mTexDisplace", 1);
	mFSFXShader->uniform("DisplaceAmount", mDisplaceAmount);
	mFSFXShader->uniform("ScrollSpeed", mScrollSpeed);
	gl::color(ColorA::white());
	gl::drawSolidRect(Rectf({ vec2(0), getWindowSize() }));
	gl::disableAlphaBlending();

	gl::enableAlphaBlending();
	gl::color(Color::white());
	gl::draw(mCloudFbo->getColorTexture(), vec2(0));
	gl::disableAlphaBlending();

	mGUI->draw();
}

void LitCloudApp::exit()
{
	mCinderDS->stop();
}

CINDER_APP_NATIVE(LitCloudApp, RendererGl{ RendererGl::Options().msaa(0) })
