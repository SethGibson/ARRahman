#ifdef _DEBUG
#pragma comment(lib, "DSAPI32.dbg.lib")
#else
#pragma comment(lib, "DSAPI32.lib")
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
#include "cinder/ImageIo.h"
#include "cinder/params/Params.h"
#include "CiDSAPI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class ChromeCloudApp : public AppNative
{
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag(MouseEvent event) override;
	void update() override;
	void draw() override;

	void exit();

	struct CloudPoint
	{
		vec3 IPosition;
		vec3 IColor;
		CloudPoint(vec3 pPos, vec3 pColor) : IPosition(pPos), IColor(pColor){}
	};

private:
	void setupGUI();
	void setupDSAPI();
	void setupMeshes();
	void setupFBOs();

	void setupCloudFBO();
	void renderCloudFBO();

	//Skybox
	geom::Cube				mSkyBox;
	gl::BatchRef			mSkyBoxBatch;
	gl::TextureCubeMapRef	mTexCube;
	gl::GlslProgRef			mSkyBoxShader;

	//PointCloud
	geom::Sphere			mCloudCube;
	gl::BatchRef			mCloudBatch;
	geom::BufferLayout		mCloudAttribs;
	gl::VboRef				mCloudData;
	gl::VboMeshRef			mCloudMesh;
	gl::GlslProgRef			mCloudShader, mScrollShader;
	gl::FboRef				mCloudFbo, mScrollFbo;
	gl::Texture2dRef		mTexCloudAlpha;
	gl::Texture2dRef		mTexScroll;
	gl::GlslProgRef			mBlurShader;

	CameraPersp				mCamera;
	MayaCamUI				mMayaCam;
	vector<CloudPoint>		mPositions;

	CinderDSRef				mCinderDS;
	ivec2					mDepthDims;

	// GUI
	float					mColorScale,
							mReflScale,
							mCloudSpeed,
							mCloudNoise;

	params::InterfaceGlRef	mGUI;
};

void ChromeCloudApp::setup()
{
	getWindow()->setSize(1280, 720);

	setupGUI();
	setupDSAPI();
	setupMeshes();
	setupFBOs();

	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 0.1, 2000.0f);
	mCamera.lookAt(vec3(0, 0, -3), vec3(0), vec3(0, 1, 0));
	mCamera.setCenterOfInterestPoint(vec3(0,0,1));
	mMayaCam.setCurrentCam(mCamera);

	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void ChromeCloudApp::setupGUI()
{
	mColorScale = 1.5f;
	mReflScale = 0.65f;
	mCloudSpeed = 0.15f;
	mCloudNoise = 0.015;
	mGUI = params::InterfaceGl::create("Params", vec2(300, 100));
	mGUI->addParam("Color Amt", &mColorScale).min(0.1f).max(3.0f).step(0.1f);
	mGUI->addParam("Env Amt", &mReflScale).min(0.1f).max(3.0f).step(0.1f);
	mGUI->addParam("Cloud Spd", &mCloudSpeed).min(0.1f).max(5.0f).step(0.1f);
	mGUI->addParam("Noise", &mCloudNoise).min(0.001f).max(1.0f).step(0.005f);
}

void ChromeCloudApp::setupDSAPI()
{
	mCinderDS = CinderDSAPI::create();
	mCinderDS->init();
	mCinderDS->initDepth(FrameSize::DEPTHSD, 60);
	mCinderDS->initRgb(FrameSize::RGBVGA, 60);
	mDepthDims = mCinderDS->getDepthSize();
	mCinderDS->start();

	getSignalShutdown().connect(std::bind(&ChromeCloudApp::exit, this));
}

void ChromeCloudApp::setupMeshes()
{
	try
	{
		mSkyBoxShader = gl::GlslProg::create(loadAsset("sky_box.vert"), loadAsset("sky_box.frag"));
	}
	catch (const gl::GlslProgExc &e)
	{
		console() << e.what() << endl;
	}

	mTexCube = gl::TextureCubeMap::create(loadImage(loadAsset("env_map.jpg")), gl::TextureCubeMap::Format().mipmap());
	mSkyBox = geom::Cube();
	mSkyBoxBatch = gl::Batch::create(mSkyBox, mSkyBoxShader);
	mSkyBoxBatch->getGlslProg()->uniform("uCubeMapTex", 0);

	try
	{
		mCloudShader = gl::GlslProg::create(loadAsset("instcloud_vert.glsl"), loadAsset("instcloud_frag.glsl"));
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
			mPositions.push_back(CloudPoint(vec3(cx, cy, 0),vec3(1)));
		}
	}

	mCloudCube = geom::Sphere().radius(0.01f).subdivisions(8);
	mCloudMesh = gl::VboMesh::create(mCloudCube);

	mCloudData = gl::Vbo::create(GL_ARRAY_BUFFER, mPositions, GL_DYNAMIC_DRAW);
	mCloudAttribs.append(geom::CUSTOM_0, 3, sizeof(CloudPoint), offsetof(CloudPoint, IPosition), 1);
	mCloudAttribs.append(geom::CUSTOM_1, 3, sizeof(CloudPoint), offsetof(CloudPoint, IColor), 1);
	
	mCloudMesh->appendVbo(mCloudAttribs, mCloudData);
	mCloudBatch = gl::Batch::create(mCloudMesh, mCloudShader, { { geom::CUSTOM_0, "iPosition" }, { geom::CUSTOM_1, "iColor" } });
	mCloudBatch->getGlslProg()->uniform("uCubeMapTex", 0);
}

void ChromeCloudApp::setupFBOs()
{
	setupCloudFBO();

}

void ChromeCloudApp::setupCloudFBO()
{

	try
	{
		mScrollShader = gl::GlslProg::create(loadAsset("vert_fbo.glsl"), loadAsset("scroll_fbo.glsl"));
	}
	catch (const gl::GlslProgExc &e)
	{
		console() << e.what() << endl;
	}


	mTexScroll = gl::Texture2d::create(loadImage(loadAsset("cloud_map.png")), gl::Texture2d::Format().wrap(GL_REPEAT).internalFormat(GL_RGBA));
	mScrollShader->uniform("mTexColor", 0);
	mScrollShader->uniform("mTexMask", 1);

	gl::Fbo::Format cFboFormat;
	gl::Texture2d::Format cTexFormat = gl::Texture2d::Format().internalFormat(GL_RGBA8);
	
	gl::Texture2dRef cCloudColor = gl::Texture2d::create(1280, 720, cTexFormat);
	gl::Texture2dRef cScrollColor = gl::Texture2d::create(1280, 720, cTexFormat);
	
	cFboFormat.attachment(GL_COLOR_ATTACHMENT0, cCloudColor);
	cFboFormat.depthTexture();
	mCloudFbo = gl::Fbo::create(1280, 720, cFboFormat);
	mTexCloudAlpha = gl::Texture2d::create(1280, 720, cTexFormat);

	cFboFormat.attachment(GL_COLOR_ATTACHMENT0, cScrollColor);
	cFboFormat.depthTexture();
	mScrollFbo = gl::Fbo::create(1280, 720, cFboFormat);
}

void ChromeCloudApp::mouseDown( MouseEvent event )
{
	mMayaCam.mouseDown(event.getPos());
}

void ChromeCloudApp::mouseDrag(MouseEvent event)
{
	mMayaCam.mouseDrag(event.getPos(), event.isLeftDown(), false, event.isRightDown());
}

void ChromeCloudApp::renderCloudFBO()
{
	mCloudFbo->bindFramebuffer();
	gl::ScopedViewport cCloudVP(vec2(0), mCloudFbo->getSize());
	gl::setMatrices(mMayaCam.getCamera());
	gl::clear(ColorA::zero());
	gl::color(ColorA::white());
	mCloudBatch->getGlslProg()->uniform("mColorAmt", mColorScale);
	mCloudBatch->getGlslProg()->uniform("mReflAmt", mReflScale);
	mCloudBatch->drawInstanced(mPositions.size());

	mCloudFbo->unbindFramebuffer();

	mScrollFbo->bindFramebuffer();
	gl::enableAlphaBlending();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());
	gl::color(ColorA::white());
	mCloudFbo->bindTexture(0);
	mTexScroll->bind(1);
	mScrollShader->bind();
	mScrollShader->uniform("mCloudSpeed", mCloudSpeed);
	mScrollShader->uniform("mCloudNoise", mCloudNoise);
	gl::drawSolidRect(Rectf({ vec2(0), mScrollFbo->getSize() }));
	mCloudFbo->unbindTexture();
	mTexScroll->unbind();
	gl::disableAlphaBlending();
	mScrollFbo->unbindFramebuffer();
}

void ChromeCloudApp::update()
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
					mPositions.push_back(CloudPoint(vec3(cx, cy, cz),vec3(cColor.r, cColor.g, cColor.b)));
				}
			}
		}
	}

	mCloudData->bufferData(mPositions.size()*sizeof(vec3), mPositions.data(), GL_DYNAMIC_DRAW);
	mCloudMesh = gl::VboMesh::create(mCloudCube);
	mCloudMesh->appendVbo(mCloudAttribs, mCloudData);
	mCloudBatch->replaceVboMesh(mCloudMesh);

	renderCloudFBO();
}

void ChromeCloudApp::draw()
{
	gl::clear(Color(0, 0, 0));
	gl::enableAlphaBlending();
	gl::color(Color::white());
	gl::setMatrices(mMayaCam.getCamera());

	gl::pushMatrices();
	gl::scale(500, 500, 500);
	mTexCube->bind();
	mSkyBoxBatch->draw();
	gl::popMatrices();

	gl::setMatricesWindow(getWindowSize());
	
	gl::draw(mScrollFbo->getColorTexture(), vec2(0));
	gl::disableAlphaBlending();

	mGUI->draw();
}

void ChromeCloudApp::exit()
{
	mCinderDS->stop();
}

CINDER_APP_NATIVE(ChromeCloudApp, RendererGl{RendererGl::Options().msaa(0)})
