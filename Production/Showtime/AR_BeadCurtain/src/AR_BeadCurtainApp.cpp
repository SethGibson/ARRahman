#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/CinderMath.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static ivec2 S_WINDOW_SIZE(960, 540);
static int S_ROW_STEP = 30;
static int S_COL_STEP = 60;


class AR_BeadCurtainApp : public App
{
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;

private:
	gl::GlslProgRef			mShader;
	gl::VboMeshRef			mBallMesh;
	gl::VboRef				mPositions;
	gl::BatchRef			mCurtain;
	gl::TextureCubeMapRef	mEnvMap;

	ivec2					mWindowSize;
	CameraPersp				mCamera;
	CameraUi				mCameraUi;
};

void AR_BeadCurtainApp::setup()
{
	getWindow()->setSize(S_WINDOW_SIZE);

	//setup shader
	mEnvMap = gl::TextureCubeMap::create(loadImage(loadAsset("textures/skybox.png")));
	mShader = gl::GlslProg::create(loadAsset("shaders/glass_ball_vert.glsl"), loadAsset("shaders/glass_ball_frag.glsl"));

	//setup geo
	vector<vec3> positions;
	vector<float> sizes;
	for (int dy = 0; dy < S_WINDOW_SIZE.y; dy += S_ROW_STEP)
	{
		float py = lmap<float>(dy, 0.0f, static_cast<float>(S_WINDOW_SIZE.y-S_ROW_STEP), -1.0f, 1.0f);
		for (int dx = 0; dx < S_WINDOW_SIZE.x; dx += S_COL_STEP)
		{
			float px = lmap<float>(dx, 0.0f, static_cast<float>(S_WINDOW_SIZE.x-S_COL_STEP), -1.7778f, 1.7778f);
			positions.push_back(vec3(static_cast<float>(px), static_cast<float>(py), 0.0f));
			sizes.push_back(randFloat(0.2f, 1.0f));
		}
	}

	
	mBallMesh = gl::VboMesh::create(geom::Sphere().radius(0.05f));
	geom::BufferLayout posAttrib, sizeAttrib;

	mPositions = gl::Vbo::create(GL_ARRAY_BUFFER, positions, GL_STATIC_DRAW);
	posAttrib.append(geom::CUSTOM_0, 3, sizeof(vec3), 0, 1);

	gl::VboRef sizeBuffer = gl::Vbo::create(GL_ARRAY_BUFFER, sizes, GL_STATIC_DRAW);
	sizeAttrib.append(geom::CUSTOM_1, 1, sizeof(float), 0, 1);

	mBallMesh->appendVbo(posAttrib, mPositions);
	mBallMesh->appendVbo(sizeAttrib, sizeBuffer);
	mCurtain = gl::Batch::create(mBallMesh, mShader, { { geom::CUSTOM_0, "iPosition" }, { geom::CUSTOM_1, "iSize" } });
	mCurtain->getGlslProg()->uniform("uCubemapSampler", 0);

	mCamera.setPerspective(45.0f,getWindowAspectRatio(), 0.1f, 100.0f);
	mCamera.lookAt(vec3(0, 0, -2.25f), vec3(0), vec3(0, 1, 0));
	mCamera.setPivotDistance(2.25f);
	mCameraUi = CameraUi(&mCamera, getWindow());
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void AR_BeadCurtainApp::mouseDown( MouseEvent event )
{
}

void AR_BeadCurtainApp::update()
{
	float eyeX = 0.25f*math<float>::sin(getElapsedFrames()*0.01f);
	float eyeY = 0.25f*math<float>::cos(getElapsedFrames()*0.01f);
	//float eyePosX = (-2.25f*math<float>::cos(M_PI / 2.0f)) - eyeOffset;
	//float eyePosZ = (-2.25f*math<float>::sin(M_PI / 2.0f)) - eyeOffset;
	mCamera.lookAt(vec3(eyeX, eyeY, -2.25f), vec3(0), vec3(0, 1, 0));

}

void AR_BeadCurtainApp::draw()
{
	float lightPosX = 10.0f*math<float>::cos(getElapsedFrames()*0.001f);
	float lightPosZ = 10.0f*math<float>::sin(getElapsedFrames()*0.001f);
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatrices(mCamera);

	mEnvMap->bind(0);
	mCurtain->getGlslProg()->uniform("uEyePos", mCamera.getEyePoint());
	mCurtain->getGlslProg()->uniform("uLightPos", vec3(lightPosX, 10, lightPosZ));
	mCurtain->getGlslProg()->uniform("uRefraction", 1.0f/2.42f);
	mCurtain->drawInstanced((S_WINDOW_SIZE.x / S_COL_STEP)*(S_WINDOW_SIZE.y / S_ROW_STEP));
	mEnvMap->unbind();
}

CINDER_APP( AR_BeadCurtainApp, RendererGl(RendererGl::Options().msaa(16)) )
