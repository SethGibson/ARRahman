#ifndef _DEBUG
#pragma comment(lib, "DSAPI.lib")
#else
#pragma comment(lib, "DSAPI.dbg.lib")
#endif
#include "VisualizerApp.h"
#include "cinder/ObjLoader.h"
#include "cinder/TriMesh.h"

#pragma region Constants
const ivec2 DEPTH_SIZE(320, 240);
const ivec2 RGB_SIZE(640, 480);
const ivec2 WINDOW_SIZE(1280, 720);
const ivec2 VP_SIZE(1280, 720);

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
const string AMB_STR_NAME =		"uAmbientStrength";
const string IMAGE_SIZE_NAME =	"uImageSize";
const string INST_POS_NAME =	"iPosition";
const string INST_UV_NAME =		"iTexCoord0";
const string INST_SIZE_NAME =	"iSize";
const string INST_MODEL_NAME =	"iModelMatrix";


const int CUBEMAP_UNIT =		0;
const int TEXTURE_UNIT =		1;
#pragma endregion

#pragma region Core Methods
void VisualizerApp::setup()
{
	getWindow()->setSize(WINDOW_SIZE);
	setupGUI();
	setupDS();
	setupScene();
	setupSkybox(	"textures/ph_cubemap.png",
					{"shaders/skybox_vertex.glsl","shaders/skybox_fragment.glsl"},
					{CUBEMAP_NAME,CUBEMAP_UNIT});

	setupPointCloud(	{"shaders/pointcloud_vertex.glsl","shaders/pointcloud_fragment.glsl"},
						{{ CUBEMAP_NAME, CUBEMAP_UNIT },{ TEXTURE_NAME, TEXTURE_UNIT }});

	setupParticles("objects/rosepetal.obj", "textures/rosepetal.png",
					{ "shaders/particle_vertex.glsl", "shaders/particle_fragment.glsl" },
					{ {TEXTURE_NAME, TEXTURE_UNIT} });
	
	setupFBOs();
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
	updateFBOs();
}

void VisualizerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 

	//draw 3d
	gl::setMatrices(mCamera);
	drawSkybox();
	drawPointCloud();
	//drawParticles();

	//draw 2d
	/*
	gl::setMatricesWindow(getWindowSize());
	drawFBO();
	if (mDrawGUI)
		mGUI->draw();
	*/
	gl::setMatricesWindow(getWindowSize());
	//drawFBO();
	gl::draw(mParticlesBlurVRT->getColorTexture(), vec2(0));
}

void VisualizerApp::cleanup()
{
	mDS->stop();
}
#pragma endregion

#pragma region Setup Methods
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

	mParamParticlesSpecularPower = 4.0f;
	mParamParticlesSpecularStrength = 2.0f;
	mParamParticlesAmbientStrength = 0.4f;
	mParamParticlesBlurUStep = 512.0f;
	mParamParticlesBlurVStep = 512.0f;

	mGUI = params::InterfaceGl::create("Settings", vec2(300, 400));
	mGUI->addSeparator();
	mGUI->addText("Point Cloud");
	mGUI->addSeparator();
	mGUI->addParam<int>("cloudRes", &mParamPointcloudStep).optionsStr("label='Resolution'");
	mGUI->addParam<float>("cloudSize", &mParamPointcloudSize).optionsStr("label='Point Size'");
	mGUI->addParam<float>("cloudSpecularPower", &mParamPointcloudSpecularPower).optionsStr("label='Highlight Size'");
	mGUI->addParam<float>("cloudSpecularStrength", &mParamPointcloudSpecularStrength).optionsStr("label='Highlight Strength'");
	mGUI->addParam<float>("cloudFresnelPower", &mParamPointcloudFresnelPower).optionsStr("label='Rimlight Size'");
	mGUI->addParam<float>("cloudFresnelStrength", &mParamPointcloudFresnelStrength).optionsStr("label='Rimlight Strength'");
	mGUI->addParam<float>("cloudReflectionStrength", &mParamPointcloudReflectionStrength).optionsStr("label='Reflection Strength'");
	mGUI->addParam<float>("cloudLightX", &mParamPointcloudLightPositionX).optionsStr("label='Light Position X'");
	mGUI->addParam<float>("cloudLightY", &mParamPointcloudLightPositionY).optionsStr("label='Light Position Y'");
	mGUI->addParam<float>("cloudLightZ", &mParamPointcloudLightPositionZ).optionsStr("label='Light Position Z'");
	mGUI->addSeparator();
	mGUI->addText("Particles");
	mGUI->addSeparator();
	mGUI->addParam<float>("particlesSpecularPower", &mParamParticlesSpecularPower).optionsStr("label='Highlight Size'");
	mGUI->addParam<float>("particlesSpecularStrength", &mParamParticlesSpecularStrength).optionsStr("label='Highlight Strength'");
	mGUI->addParam<float>("particlesAmbientStrength", &mParamParticlesAmbientStrength).optionsStr("label='Ambient Strength'");
	mGUI->addParam<float>("particlesBlurUStep", &mParamParticlesBlurUStep).optionsStr("label='H Blur Width'");
	mGUI->addParam<float>("particlesBlurVStep", &mParamParticlesBlurVStep).optionsStr("label='V Blur Width'");

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
	mPointcloudInstanceAttribs.append(geom::CUSTOM_1, 1, sizeof(Particle), offsetof(Particle, PSize), 1);
	mPointcloudInstanceAttribs.append(geom::CUSTOM_2, 2, sizeof(Particle), offsetof(Particle, PUV), 1);


	gl::VboMeshRef pointcloudMesh = gl::VboMesh::create(geom::Sphere());
	pointcloudMesh->appendVbo(mPointcloudInstanceAttribs, mPointcloudInstanceData);

	mPointcloudShader = gl::GlslProg::create(loadAsset(pShaders.first), loadAsset(pShaders.second));
	mPointcloudBatch = gl::Batch::create(	pointcloudMesh, mPointcloudShader,
											{
												{ geom::CUSTOM_0, INST_POS_NAME },
												{ geom::CUSTOM_1, INST_SIZE_NAME },
												{ geom::CUSTOM_2, INST_UV_NAME },
											});

	for (auto u : pSamplerUniforms)
		mPointcloudBatch->getGlslProg()->uniform(u.first, u.second);

	mPointcloudTexture = gl::Texture2d::create(RGB_SIZE.x, RGB_SIZE.y);
}

void VisualizerApp::setupParticles(string pObjFile, string pTextureFile, pair<string, string> pShaders, vector<pair<string,int>> pSamplerUniforms)
{
	mNoOfFramesBeforeSpawingParticles = 200;
	mNoOfFramesElapsed = 200;
	mNumberOfParticlesSpawned = 0;
	mTimeToSpawnParticles = 0;
	mMaxParticleCount = 5000;
	mIterator = ivec3();
	mDecrementer = ivec3();
	mParticlesPerlin = Perlin();

	ObjLoader rawMesh(loadAsset(pObjFile));
	TriMeshRef meshSrc = TriMesh::create(rawMesh);
	if (!rawMesh.getAvailableAttribs().count(geom::NORMAL))
		meshSrc->recalculateNormals();

	gl::VboMeshRef particleMesh = gl::VboMesh::create(*meshSrc);
	
	mParticlesPoints.push_back(Particle(vec3(0)));
	mParticlesInstanceData = gl::Vbo::create(GL_ARRAY_BUFFER, mParticlesPoints, GL_DYNAMIC_DRAW);
	mParticlesInstanceAttribs.append(geom::CUSTOM_0, 3, sizeof(Particle), offsetof(Particle, PPosition), 1);
	mParticlesInstanceAttribs.append(geom::CUSTOM_1, 1, sizeof(Particle), offsetof(Particle, PSize), 1);
	mParticlesInstanceAttribs.append(geom::CUSTOM_3, 16, sizeof(Particle), offsetof(Particle, PModelMatrix), 1);
	particleMesh->appendVbo(mParticlesInstanceAttribs, mParticlesInstanceData);
	mParticlesShader = gl::GlslProg::create(loadAsset(pShaders.first), loadAsset(pShaders.second));
	mParticlesBatch = gl::Batch::create(particleMesh, mParticlesShader,
										{
											{ geom::CUSTOM_0, INST_POS_NAME },
											{ geom::CUSTOM_1, INST_SIZE_NAME },
											{ geom::CUSTOM_3, INST_MODEL_NAME }
										});

	for (auto u : pSamplerUniforms)
		mParticlesBatch->getGlslProg()->uniform(u.first, u.second);

	mParticlesTexture = gl::Texture2d::create(loadImage(loadAsset(pTextureFile)));
}

void VisualizerApp::setupFBOs()
{
	mParticlesBaseRT = gl::Fbo::create(VP_SIZE.x, VP_SIZE.y, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));
	mParticlesHighPassRT = gl::Fbo::create(VP_SIZE.x, VP_SIZE.y, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));
	mParticlesBlurURT = gl::Fbo::create(VP_SIZE.x, VP_SIZE.y, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));
	mParticlesBlurVRT = gl::Fbo::create(VP_SIZE.x, VP_SIZE.y, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));

	mParticlesHighpassShader = gl::GlslProg::create(loadAsset("shaders/passthru_vertex.glsl"), loadAsset("shaders/highpass_fragment.glsl"));
	mParticlesHighpassShader->uniform(TEXTURE_NAME, TEXTURE_UNIT);

	mParticlesBlurUShader = gl::GlslProg::create(loadAsset("shaders/passthru_vertex.glsl"), loadAsset("shaders/blur_u_fragment.glsl"));
	mParticlesBlurUShader->uniform(TEXTURE_NAME, TEXTURE_UNIT);
	mParticlesBlurUShader->uniform(IMAGE_SIZE_NAME, vec2(1280,720));


	mParticlesBlurVShader = gl::GlslProg::create(loadAsset("shaders/passthru_vertex.glsl"), loadAsset("shaders/blur_v_fragment.glsl"));
	mParticlesBlurVShader->uniform(TEXTURE_NAME, TEXTURE_UNIT);
	mParticlesBlurVShader->uniform(IMAGE_SIZE_NAME, vec2(1280, 720));
}

#pragma endregion

#pragma region Update Methods
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
				if (z > 0.0f && z < 1500.0f)
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
	mTimeToSpawnParticles--;

	if (mTimeToSpawnParticles <= 0)
	{
		if (mParticlesPoints.size()<mMaxParticleCount)
		{
			int noOfParticlesPerSpawn = 10;
			int TotalNumberOfParticles = 500;
			for (int i = 0; i < noOfParticlesPerSpawn; i++)
				mParticlesPoints.push_back(Particle(vec3(Rand::randFloat(-50.0f, 50.0f), Rand::randFloat(70.0f, 30.0f), 100)));

			mNumberOfParticlesSpawned = mNumberOfParticlesSpawned + 2 * noOfParticlesPerSpawn;
			if (mNumberOfParticlesSpawned > TotalNumberOfParticles)
			{
				//reset after spawning
				mNumberOfParticlesSpawned = 0;
				mIterator.x = Rand::randInt(-500, -300);
				mIterator.z = Rand::randInt(300, 700);
				mDecrementer.x = Rand::randInt(700, 1000);
				mDecrementer.z = Rand::randInt(400, 500);
				mTimeToSpawnParticles = Rand::randInt(500, 550);
			}
		}
	}

	float elapsedTime = (float)getElapsedSeconds();
	vec3 billboardUpVector, billboardRightVector;
	mCamera.getBillboardVectors(&billboardRightVector, &billboardUpVector);
	for (auto pit = mParticlesPoints.begin(); pit != mParticlesPoints.end();)
	{
		pit->Step(elapsedTime, mParticlesPerlin, billboardRightVector);
		if (pit->Age <= 0)
			pit = mParticlesPoints.erase(pit);
		else
			++pit;
	}

	mParticlesInstanceData->bufferData(mParticlesPoints.size()*sizeof(Particle), mParticlesPoints.data(), GL_DYNAMIC_DRAW);
}

void VisualizerApp::updateFBOs()
{
	Rectf windowRect({ vec2(), VP_SIZE });
	gl::ScopedViewport renderView(VP_SIZE);

	mParticlesBaseRT->bindFramebuffer();
	gl::setMatrices(mCamera);
	gl::clear(ColorA::zero());
	drawParticles();
	mParticlesBaseRT->unbindFramebuffer();

	mParticlesHighPassRT->bindFramebuffer();
	mParticlesBaseRT->bindTexture(TEXTURE_UNIT);
	mParticlesHighpassShader->bind();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());
	gl::color(ColorA::white());
	gl::drawSolidRect(windowRect);
	mParticlesBaseRT->unbindTexture(TEXTURE_UNIT);
	mParticlesHighPassRT->unbindFramebuffer();

	mParticlesBlurURT->bindFramebuffer();
	mParticlesHighPassRT->bindTexture(TEXTURE_UNIT);
	mParticlesBlurUShader->bind();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());
	gl::color(ColorA::white());
	gl::drawSolidRect(windowRect);
	mParticlesHighPassRT->unbindTexture(TEXTURE_UNIT);
	mParticlesBlurURT->unbindFramebuffer();

	mParticlesBlurVRT->bindFramebuffer();
	mParticlesBlurURT->bindTexture(TEXTURE_UNIT);
	mParticlesBlurVShader->bind();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());
	gl::color(ColorA::white());
	gl::drawSolidRect(windowRect);
	mParticlesBlurURT->unbindTexture(TEXTURE_UNIT);
	mParticlesBlurVRT->unbindFramebuffer();
}

#pragma endregion

#pragma region Draw Methods
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
	gl::enableDepthRead();
	mSkyboxTexture->bind(CUBEMAP_UNIT);
	mPointcloudTexture->bind(TEXTURE_UNIT);
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
	gl::disableDepthRead();
}

void VisualizerApp::drawParticles()
{
	gl::enableDepthRead();
	mParticlesTexture->bind(TEXTURE_UNIT);
	mParticlesBatch->getGlslProg()->uniform(LIGHT_POS_NAME, vec3(mParamPointcloudLightPositionX, mParamPointcloudLightPositionY, mParamPointcloudLightPositionZ));
	mParticlesBatch->getGlslProg()->uniform(VIEW_DIR_NAME, mCamera.getViewDirection());
	mParticlesBatch->getGlslProg()->uniform(SPEC_POW_NAME, mParamParticlesSpecularPower);
	mParticlesBatch->getGlslProg()->uniform(SPEC_STR_NAME, mParamParticlesSpecularStrength);
	mParticlesBatch->getGlslProg()->uniform(AMB_STR_NAME, mParamParticlesAmbientStrength);
	mParticlesBatch->drawInstanced(mParticlesPoints.size());
	mParticlesTexture->unbind();
	gl::disableDepthRead();
}

void VisualizerApp::drawFBO()
{
	gl::disableDepthRead();
	gl::enableAdditiveBlending();
	gl::draw(mParticlesBlurVRT->getColorTexture(), Rectf({vec2(0), vec2(WINDOW_SIZE)}));
	gl::disableAlphaBlending();
	gl::enableDepthRead();
}
#pragma endregion

CINDER_APP( VisualizerApp, RendererGl )