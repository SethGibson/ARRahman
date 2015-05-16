#ifndef _DEBUG
#pragma comment(lib, "DSAPI.lib")
#else
#pragma comment(lib, "DSAPI.dbg.lib")
#endif
#include "VisualizerApp.h"

const ivec2 DEPTH_SIZE(320, 240);
const ivec2 RGB_SIZE(640, 480);

const string CUBEMAP_NAME =		"uCubemapSampler";
const string TEXTURE_NAME =		"uTextureSampler";
const string MAT_MODEL_NAME =	"uModelMatrix";
const string MAT_PROJ_NAME =	"uProjMatrix";
const string LIGHT_POS_NAME =	"uLightPosition";
const string VIEW_DIR_NAME =	"uViewDirection";
const string SPEC_POW_NAME =	"uSpecularPower";
const string SPEC_STR_NAME =	"uSpecularStrength";
const string FRES_POW_NAME =	"uFresnelPower";
const string FRES_STR_NAME =	"uFresnelStrength";
const string REFL_STR_NAME =	"uReflectionStrength";
const string INST_POS_NAME =	"iPosition";
const string INST_UV_NAME =		"iTexCoord0";
const string INST_SIZE_NAME =	"iSize";


const int CUBEMAP_UNIT =		0;
const int TEXTURE_UNIT =		1;

void VisualizerApp::setup()
{
	getWindow()->setSize(1280, 720);
	setupGUI();
	setupDS();
	setupScene();
	setupSkybox(	"textures/ph_cubemap.png",
					{"shaders/skybox_vertex.glsl","shaders/skybox_fragment.glsl"},
					{CUBEMAP_NAME,CUBEMAP_UNIT});

	setupPointCloud(	{"shaders/pointcloud_vertex.glsl","shaders/pointcloud_fragment.glsl"},
						{{ CUBEMAP_NAME, CUBEMAP_UNIT },{ TEXTURE_NAME, TEXTURE_UNIT }});
}

void VisualizerApp::keyDown(KeyEvent pEvent)
{
	if (pEvent.getChar() == 'd')
		mDrawGUI = !mDrawGUI;
}
void VisualizerApp::update()
{
	updatePointCloud();
	updateParticles();
}

void VisualizerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	//draw 3d
	gl::setMatrices(mCamera);
	drawSkybox();
	drawPointCloud();
	drawParticles();

	//draw 2d
	gl::setMatricesWindow(getWindowSize());
	if (mDrawGUI)
		mGUI->draw();
}

void VisualizerApp::cleanup()
{
	mDS->stop();
}

// Setup Methods
void VisualizerApp::setupDS()
{
	mDS = CinderDSAPI::create();
	mDS->init();
	mDS->initDepth(FrameSize::DEPTHQVGA, 60);
	mDS->initRgb(FrameSize::RGBVGA, 60);
	mDS->start();
}

void VisualizerApp::setupGUI()
{
	mDrawGUI = true;
	mParamPointcloudStep = 2;
	mParamPointcloudSize = 3.0f;
	mParamPointcloudSpecularPower = 8.0f;
	mParamPointcloudSpecularStrength = 0.5f;
	mParamPointcloudFresnelPower = 8.0f;
	mParamPointcloudFresnelStrength = 1.0f;
	mParamPointcloudReflectionStrength = 1.0f;
	mParamPointcloudLightPositionX = 0.0f;
	mParamPointcloudLightPositionY = 500.0f;
	mParamPointcloudLightPositionZ = 0.0f;

	mGUI = params::InterfaceGl::create("Settings", vec2(300, 300));
	mGUI->addSeparator();
	mGUI->addText("Point Cloud");
	mGUI->addSeparator();
	mGUI->addParam<int>("Resolution", &mParamPointcloudStep);
	mGUI->addParam<float>("Point Size", &mParamPointcloudSize);
	mGUI->addParam<float>("Highlight Size", &mParamPointcloudSpecularPower);
	mGUI->addParam<float>("Highlight Strength", &mParamPointcloudSpecularStrength);
	mGUI->addParam<float>("Rimlight Size", &mParamPointcloudFresnelPower);
	mGUI->addParam<float>("Rimlight Strength", &mParamPointcloudFresnelStrength);
	mGUI->addParam<float>("Reflection Strength", &mParamPointcloudReflectionStrength);
	mGUI->addParam<float>("Light Position X", &mParamPointcloudLightPositionX);
	mGUI->addParam<float>("Light Position Y", &mParamPointcloudLightPositionY);
	mGUI->addParam<float>("Light Position Z", &mParamPointcloudLightPositionZ);
}
void VisualizerApp::setupScene()
{
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 100.0f, 5000.0f);
	mCamera.lookAt(vec3(0), vec3(0, 0, 1000), vec3(0, -1, 0));
	mCamera.setPivotDistance(500.0f);

	mCameraUi = CameraUi(&mCamera, getWindow());
}

void VisualizerApp::setupSkybox(string pTextureFile, pair<string,string> pShaders, pair<string, int> pSamplerUniform)
{
	mSkyboxTexture = gl::TextureCubeMap::create(loadImage(loadAsset(pTextureFile)), gl::TextureCubeMap::Format().mipmap().internalFormat(GL_RGBA8));
	mSkyboxShader = gl::GlslProg::create(loadAsset(pShaders.first), loadAsset(pShaders.second));
	mSkyboxBatch = gl::Batch::create(geom::Cube(), mSkyboxShader);
	mSkyboxBatch->getGlslProg()->uniform(pSamplerUniform.first, pSamplerUniform.second);
}

void VisualizerApp::setupPointCloud(pair<string, string> pShaders, vector<pair<string,int>> pSamplerUniforms)
{
	mPointcloudPoints.push_back(Particle(vec3(0), vec2(0), mParamPointcloudSize));
	mPointcloudInstanceData = gl::Vbo::create(GL_ARRAY_BUFFER, mParticlesPoints, GL_DYNAMIC_DRAW);
	mPointcloudInstanceAttribs.append(geom::CUSTOM_0, 3, sizeof(Particle), offsetof(Particle, PPosition), 1);
	mPointcloudInstanceAttribs.append(geom::CUSTOM_1, 2, sizeof(Particle), offsetof(Particle, PUV), 1);
	mPointcloudInstanceAttribs.append(geom::CUSTOM_2, 1, sizeof(Particle), offsetof(Particle, PSize), 1);

	gl::VboMeshRef pointcloudMesh = gl::VboMesh::create(geom::Sphere());
	pointcloudMesh->appendVbo(mPointcloudInstanceAttribs, mPointcloudInstanceData);

	mPointcloudShader = gl::GlslProg::create(loadAsset(pShaders.first), loadAsset(pShaders.second));
	mPointcloudBatch = gl::Batch::create(	pointcloudMesh,
											mPointcloudShader,
											{
												{ geom::CUSTOM_0, INST_POS_NAME },
												{ geom::CUSTOM_1, INST_UV_NAME },
												{ geom::CUSTOM_2, INST_SIZE_NAME }
											});

	for (auto u : pSamplerUniforms)
		mPointcloudBatch->getGlslProg()->uniform(u.first, u.second);

	mPointcloudTexture = gl::Texture2d::create(RGB_SIZE.x, RGB_SIZE.y);
}

void VisualizerApp::setupParticles()
{

}

// Update Methods
void VisualizerApp::updatePointCloud()
{
	mDS->update();
	mPointcloudPoints.clear();
	mPointcloudTexture->update(mDS->getRgbFrame());
	
	Channel16u depthChannel = mDS->getDepthFrame();
	Channel16u::Iter depthIter = depthChannel.getIter();
	while (depthIter.line())
	{
		while (depthIter.pixel())
		{
			int x = depthIter.x();
			int y = depthIter.y();
			if (x % mParamPointcloudStep == 0 && y % mParamPointcloudStep == 0)
			{
				float z = (float)depthIter.v();
				if (z > 100.0f && z < 1500.0f)
				{
					vec3 worldPosition = mDS->getDepthSpacePoint(static_cast<float>(x), static_cast<float>(y), z);
					vec2 texCoord = mDS->getColorCoordsFromDepthSpace(worldPosition);
					mPointcloudPoints.push_back(Particle(worldPosition, texCoord, mParamPointcloudSize));
				}
			}
		}
	}

	mPointcloudInstanceData->bufferData(mPointcloudPoints.size()*sizeof(Particle), mPointcloudPoints.data(), GL_DYNAMIC_DRAW);
}

void VisualizerApp::updateParticles()
{

}

// Draw Methods
void VisualizerApp::drawSkybox()
{
	glDisable(GL_DEPTH_TEST);
	gl::pushMatrices();
	gl::scale(vec3(1, -1, 1));
	mSkyboxTexture->bind(CUBEMAP_UNIT);
	mSkyboxBatch->draw();
	mSkyboxTexture->unbind();
	gl::popMatrices();
	glEnable(GL_DEPTH_TEST);
}

void VisualizerApp::drawPointCloud()
{
	mSkyboxTexture->bind(CUBEMAP_UNIT);
	mPointcloudTexture->bind(TEXTURE_UNIT);
	mPointcloudBatch->getGlslProg()->uniform(TEXTURE_NAME, 1);
	mPointcloudBatch->getGlslProg()->uniform(LIGHT_POS_NAME, vec3(mParamPointcloudLightPositionX, mParamPointcloudLightPositionY, mParamPointcloudLightPositionZ));
	mPointcloudBatch->getGlslProg()->uniform(VIEW_DIR_NAME, mCamera.getViewDirection());
	mPointcloudBatch->getGlslProg()->uniform(SPEC_POW_NAME, mParamPointcloudSpecularPower);
	mPointcloudBatch->getGlslProg()->uniform(SPEC_STR_NAME, mParamPointcloudSpecularStrength);
	mPointcloudBatch->getGlslProg()->uniform(FRES_POW_NAME, mParamPointcloudFresnelPower);
	mPointcloudBatch->getGlslProg()->uniform(FRES_STR_NAME, mParamPointcloudFresnelStrength);
	mPointcloudBatch->getGlslProg()->uniform(REFL_STR_NAME, mParamPointcloudReflectionStrength);
	mPointcloudBatch->drawInstanced(mPointcloudPoints.size());
	mPointcloudTexture->unbind();
	mSkyboxTexture->unbind();
}

void VisualizerApp::drawParticles()
{

}

CINDER_APP( VisualizerApp, RendererGl )
