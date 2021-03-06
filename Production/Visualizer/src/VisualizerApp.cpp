#ifndef _DEBUG
#pragma comment(lib, "DSAPI32.lib")
#else
#pragma comment(lib, "DSAPI32.dbg.lib")
#endif
#include "VisualizerApp.h"
#include "cinder/ObjLoader.h"
#include "cinder/TriMesh.h"


#pragma region Constants

const ivec2 PRODUCTION_SIZE(1920, 1080);
const string PRODUCTION_FILENAME = "production.mov";

const int MAX_PARTICLES = 5000;
const int PARTICLE_BATCH_SIZE = 3;
const ivec2 DEPTH_SIZE(320, 240);
const ivec2 RGB_SIZE(640, 480);
const ivec2 WINDOW_SIZE(1280, 720);
const ivec2 VP_SIZE(1280, 720);

const string CUBEMAP_NAME =			"uCubemapSampler";
const string TEXTURE_NAME =			"uTextureSampler";
const string MAT_MODEL_NAME =		"uModelMatrix";
const string MAT_PROJ_NAME =		"uProjMatrix";
const string LIGHT_POS_NAME =		"uLightPosition";
const string VIEW_DIR_NAME =		"uViewDirection";
const string TEX_MIN_NAME =			"uTextureMinimum";
const string SPEC_POW_NAME =		"uSpecularPower";
const string SPEC_STR_NAME =		"uSpecularStrength";
const string FRES_POW_NAME =		"uFresnelPower";
const string FRES_STR_NAME =		"uFresnelStrength";
const string REFL_STR_NAME =		"uReflectionStrength";
const string AMB_STR_NAME =			"uAmbientStrength";
const string MOVIE_SIZE_NAME =		"uMovieSize";
const string BLUR_SIZE_NAME =		"uBlurSize";
const string BLUR_AXIS_NAME =		"uBlurAxis";
const string BLUR_STR_NAME =		"uBlurStrength";
const string INST_POS_NAME =		"iPosition";
const string INST_UV_NAME =			"iTexCoord0";
const string INST_SIZE_NAME =		"iSize";
const string INST_MODEL_NAME =		"iModelMatrix";
const string INST_ALPHA_NAME =		"iAlpha";
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
	if (pEvent.getChar() == 'f')
	{
		if (getWindow()->isFullScreen())
		{
			getWindow()->setFullScreen(false);
			getWindow()->setSize(WINDOW_SIZE);
		}
		else
		{
			getWindow()->setFullScreen(true);
			getWindow()->setSize(PRODUCTION_SIZE);
		}
	}
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

	mParamPointcloudMinDepth = 100.0f,
	mParamPointcloudMaxDepth = 2000.0f,
	mParamPointcloudStep = 1;
	mParamPointcloudSize = 5.0f; //if spheres, 1.5f, if cubes, either 2.0f or something large for overlap (like 5.0f), depending on camera angle.
	mParamPointcloudTextureMinR = 0.025f;
	mParamPointcloudTextureMinG = 0.025f;
	mParamPointcloudTextureMinB = 0.025f;
	mParamPointcloudSpecularPower = 8.0f;
	mParamPointcloudSpecularStrength = 0.0f;
	mParamPointcloudFresnelPower = 2.0f;
	mParamPointcloudFresnelStrength = 1.0f;
	mParamPointcloudReflectionStrength = 1.0f;
	mParamPointcloudAmbientStrength = 1.0f;
	mParamPointcloudLightPositionX = 0.0f;
	mParamPointcloudLightPositionY = 500.0f;
	mParamPointcloudLightPositionZ = 0.0f;

	mParamParticlesSpecularPower = 4.0f;
	mParamParticlesSpecularStrength = 2.0f;
	mParamParticlesAmbientStrength = 0.4f;
	mParamParticlesBlurSizeU = 1.0f;
	mParamParticlesBlurSizeV = 1.0f;
	mParamParticlesBlurStrength = 1.0;

	mGUI = params::InterfaceGl::create("Settings", vec2(300, 600));
	mGUI->addSeparator();
	mGUI->addText("Point Cloud");
	mGUI->addSeparator();
	mGUI->addParam<float>("cloudMinDepth", &mParamPointcloudMinDepth).optionsStr("label='Min Depth'");
	mGUI->addParam<float>("cloudMaxDepth", &mParamPointcloudMaxDepth).optionsStr("label='Max Depth'");
	mGUI->addParam<int>("cloudRes", &mParamPointcloudStep).optionsStr("label='Resolution'");
	mGUI->addParam<float>("cloudSize", &mParamPointcloudSize).optionsStr("label='Point Size'");
	mGUI->addParam<float>("cloudTexMinR", &mParamPointcloudTextureMinR).optionsStr("label='Texture Min R'");
	mGUI->addParam<float>("cloudTexMinG", &mParamPointcloudTextureMinG).optionsStr("label='Texture Min G'");
	mGUI->addParam<float>("cloudTexMinB", &mParamPointcloudTextureMinB).optionsStr("label='Texture Min B'");
	mGUI->addParam<float>("cloudSpecularPower", &mParamPointcloudSpecularPower).optionsStr("label='Highlight Size'");
	mGUI->addParam<float>("cloudSpecularStrength", &mParamPointcloudSpecularStrength).optionsStr("label='Highlight Strength'");
	mGUI->addParam<float>("cloudFresnelPower", &mParamPointcloudFresnelPower).optionsStr("label='Rimlight Size'");
	mGUI->addParam<float>("cloudFresnelStrength", &mParamPointcloudFresnelStrength).optionsStr("label='Rimlight Strength'");
	mGUI->addParam<float>("cloudReflectionStrength", &mParamPointcloudReflectionStrength).optionsStr("label='Reflection Strength'");
	mGUI->addParam<float>("cloudAmbientStrength", &mParamPointcloudAmbientStrength).optionsStr("label='Ambient Strength'");
	mGUI->addParam<float>("cloudLightX", &mParamPointcloudLightPositionX).optionsStr("label='Light Position X'");
	mGUI->addParam<float>("cloudLightY", &mParamPointcloudLightPositionY).optionsStr("label='Light Position Y'");
	mGUI->addParam<float>("cloudLightZ", &mParamPointcloudLightPositionZ).optionsStr("label='Light Position Z'");
	mGUI->addSeparator();
	mGUI->addText("Particles");
	mGUI->addSeparator();
	mGUI->addParam<float>("particlesSpecularPower", &mParamParticlesSpecularPower).optionsStr("label='Highlight Size'");
	mGUI->addParam<float>("particlesSpecularStrength", &mParamParticlesSpecularStrength).optionsStr("label='Highlight Strength'");
	mGUI->addParam<float>("particlesAmbientStrength", &mParamParticlesAmbientStrength).optionsStr("label='Ambient Strength'");
	mGUI->addParam<float>("particlesBlurSizeU", &mParamParticlesBlurSizeU).optionsStr("label='Blur Width'");
	mGUI->addParam<float>("particlesBlurSizeV", &mParamParticlesBlurSizeV).optionsStr("label='Blur Height'");
	mGUI->addParam<float>("particlesBlurStrength", &mParamParticlesBlurStrength).optionsStr("label='Blur Strength'");

}
void VisualizerApp::setupScene()
{
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 100.0f, 5000.0f);
	mCamera.lookAt(vec3(0), vec3(0, 0, 1000), vec3(0, -1, 0));
	mCamera.setPivotDistance(500.0f);

	mCameraUi = CameraUi(&mCamera, getWindow());
	gl::enableDepthRead();
	gl::disableDepthWrite();
	gl::enableAlphaBlending();
}

void VisualizerApp::setupPointCloud(pair<string, string> pShaders, vector<pair<string,int>> pSamplerUniforms)
{
	mPointcloudPoints.push_back(Particle(vec3(0), vec2(0), mParamPointcloudSize));
	mPointcloudInstanceData = gl::Vbo::create(GL_ARRAY_BUFFER, mParticlesPoints, GL_DYNAMIC_DRAW);
	mPointcloudInstanceAttribs.append(geom::CUSTOM_0, 3, sizeof(Particle), offsetof(Particle, PPosition), 1);
	mPointcloudInstanceAttribs.append(geom::CUSTOM_1, 1, sizeof(Particle), offsetof(Particle, PSize), 1);
	mPointcloudInstanceAttribs.append(geom::CUSTOM_2, 2, sizeof(Particle), offsetof(Particle, PUV), 1);


	gl::VboMeshRef pointcloudMesh = gl::VboMesh::create(geom::Cube());
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
	mParticlesInstanceAttribs.append(geom::CUSTOM_4, 1, sizeof(Particle), offsetof(Particle, PAlpha), 1);
	particleMesh->appendVbo(mParticlesInstanceAttribs, mParticlesInstanceData);
	mParticlesShader = gl::GlslProg::create(loadAsset(pShaders.first), loadAsset(pShaders.second));
	mParticlesBatch = gl::Batch::create(particleMesh, mParticlesShader,
										{
											{ geom::CUSTOM_0, INST_POS_NAME },
											{ geom::CUSTOM_1, INST_SIZE_NAME },
											{ geom::CUSTOM_3, INST_MODEL_NAME },
											{ geom::CUSTOM_4, INST_ALPHA_NAME }
										});

	for (auto u : pSamplerUniforms)
		mParticlesBatch->getGlslProg()->uniform(u.first, u.second);

	mParticlesTexture = gl::Texture2d::create(loadImage(loadAsset(pTextureFile)));
}

void VisualizerApp::setupFBOs()
{
	mParticlesBaseRT = gl::Fbo::create(VP_SIZE.x, VP_SIZE.y, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));
	mParticlesBlurURT = gl::Fbo::create(VP_SIZE.x, VP_SIZE.y, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));
	mParticlesBlurVRT = gl::Fbo::create(VP_SIZE.x, VP_SIZE.y, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGBA32F).dataType(GL_FLOAT)));

	mParticlesBlurShader = gl::GlslProg::create(loadAsset("shaders/blur_vertex.glsl"), loadAsset("shaders/blur_fragment.glsl"));
	mParticlesBlurShader->uniform(TEXTURE_NAME, TEXTURE_UNIT);
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
				if (z > mParamPointcloudMinDepth && z < mParamPointcloudMaxDepth)
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
	if (getElapsedFrames() % 10 == 0 || getElapsedFrames() < 10)
	{

		for (int i = 0; i < PARTICLE_BATCH_SIZE; ++i)
		{
			if (mParticlesPoints.size() < MAX_PARTICLES)
			{
				vec3 newPosition = vec3(randFloat(-100.0f, 100.0f), randFloat(-150.0f, -200.0f), randFloat(600.0f, 700.0f));
				mParticlesPoints.push_back(Particle(newPosition));
			}
			else
			{
				break;
			}
		}
	}

	float elapsedTime = (float)getElapsedSeconds();
	for (auto pit = mParticlesPoints.begin(); pit != mParticlesPoints.end();)
	{
		pit->Step(elapsedTime, mParticlesPerlin);
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

	mParticlesBlurURT->bindFramebuffer();
	mParticlesBaseRT->bindTexture(TEXTURE_UNIT);
	mParticlesBlurShader->bind();
	mParticlesBlurShader->uniform(BLUR_SIZE_NAME, vec2(mParamParticlesBlurSizeU, mParamParticlesBlurSizeV));
	mParticlesBlurShader->uniform(BLUR_AXIS_NAME, vec2(1.0, 0.0));
	mParticlesBlurShader->uniform(BLUR_STR_NAME, mParamParticlesBlurStrength);
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());
	gl::color(ColorA::white());
	gl::drawSolidRect(windowRect);
	mParticlesBaseRT->unbindTexture(TEXTURE_UNIT);
	mParticlesBlurURT->unbindFramebuffer();

	mParticlesBlurVRT->bindFramebuffer();
	mParticlesBlurURT->bindTexture(TEXTURE_UNIT);
	mParticlesBlurShader->bind();
	mParticlesBlurShader->uniform(BLUR_SIZE_NAME, vec2(mParamParticlesBlurSizeU, mParamParticlesBlurSizeV));
	mParticlesBlurShader->uniform(BLUR_AXIS_NAME, vec2(0.0, 1.0));
	mParticlesBlurShader->uniform(BLUR_STR_NAME, mParamParticlesBlurStrength);
	gl::setMatricesWindow(getWindowSize());
	gl::clear(ColorA::zero());
	gl::color(ColorA::white());
	gl::drawSolidRect(windowRect);
	mParticlesBlurURT->unbindTexture(TEXTURE_UNIT);
	mParticlesBlurVRT->unbindFramebuffer();
}
#pragma endregion

#pragma region Draw Methods
void VisualizerApp::drawPointCloud()
{
	gl::enableDepthRead();
	if (mBackgroundTexture)
		mBackgroundTexture->bind(CUBEMAP_UNIT);
	mPointcloudTexture->bind(TEXTURE_UNIT);
	mPointcloudBatch->getGlslProg()->uniform(LIGHT_POS_NAME, vec3(mParamPointcloudLightPositionX, mParamPointcloudLightPositionY, mParamPointcloudLightPositionZ));
	mPointcloudBatch->getGlslProg()->uniform(TEX_MIN_NAME, vec3(mParamPointcloudTextureMinR, mParamPointcloudTextureMinG, mParamPointcloudTextureMinB));
	mPointcloudBatch->getGlslProg()->uniform(VIEW_DIR_NAME, mCamera.getEyePoint());
	mPointcloudBatch->getGlslProg()->uniform(SPEC_POW_NAME, mParamPointcloudSpecularPower);
	mPointcloudBatch->getGlslProg()->uniform(SPEC_STR_NAME, mParamPointcloudSpecularStrength);
	mPointcloudBatch->getGlslProg()->uniform(FRES_POW_NAME, mParamPointcloudFresnelPower);
	mPointcloudBatch->getGlslProg()->uniform(FRES_STR_NAME, mParamPointcloudFresnelStrength);
	mPointcloudBatch->getGlslProg()->uniform(REFL_STR_NAME, mParamPointcloudReflectionStrength);
	mPointcloudBatch->getGlslProg()->uniform(AMB_STR_NAME, mParamPointcloudAmbientStrength);
	mPointcloudBatch->getGlslProg()->uniform(MOVIE_SIZE_NAME, mBackgroundSize);
	mPointcloudBatch->drawInstanced(mPointcloudPoints.size());
	mPointcloudTexture->unbind();
	if (mBackgroundTexture)
		mBackgroundTexture->unbind();
	gl::disableDepthRead();
}

void VisualizerApp::drawParticles()
{
	mParticlesTexture->bind(TEXTURE_UNIT);
	mParticlesBatch->getGlslProg()->uniform(LIGHT_POS_NAME, vec3(mParamPointcloudLightPositionX, mParamPointcloudLightPositionY, mParamPointcloudLightPositionZ));
	mParticlesBatch->getGlslProg()->uniform(VIEW_DIR_NAME, mCamera.getViewDirection());
	mParticlesBatch->getGlslProg()->uniform(SPEC_POW_NAME, mParamParticlesSpecularPower);
	mParticlesBatch->getGlslProg()->uniform(SPEC_STR_NAME, mParamParticlesSpecularStrength);
	mParticlesBatch->getGlslProg()->uniform(AMB_STR_NAME, mParamParticlesAmbientStrength);
	mParticlesBatch->drawInstanced(mParticlesPoints.size());
	mParticlesTexture->unbind();
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