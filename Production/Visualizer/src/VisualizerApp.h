#ifndef _VISUALIZER_H_
#define _VISUALIZER_H_
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "CiDSAPI.h"
#include "Particle.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace CinderDS;

class VisualizerApp : public App
{
public:
	void setup()					override;
	void keyDown(KeyEvent pEvent)	override;
	void update()					override;
	void draw()						override;
	void cleanup()					override;

private:
	void setupGUI();
	void setupDS();
	void setupScene();
	void setupSkybox(string pTextureFile, pair<string, string> pShaders, pair<string,int> pSamplerUniform);
	void setupPointCloud(pair<string, string> pShaders, vector<pair<string,int>> pSamplerUniforms);
	void setupParticles(string pObjFile, string pTextureFile, pair<string, string> pShaders, vector<pair<string, int>> pSamplerUniforms);
	void setupFBOs();

	void updatePointCloud();
	void updateParticles();
	void updateFBOs();

	void drawSkybox();
	void drawPointCloud();
	void drawParticles();
	void drawFBO();

	CinderDSRef		mDS;

	//Scene
	CameraPersp				mCamera;
	CameraUi				mCameraUi;

	//Skybox
	gl::BatchRef			mSkyboxBatch;
	gl::GlslProgRef			mSkyboxShader;
	gl::TextureCubeMapRef	mSkyboxTexture;

	//Point Cloud
	vector<Particle>		mPointcloudPoints;
	gl::BatchRef			mPointcloudBatch;
	gl::VboRef				mPointcloudInstanceData;
	geom::BufferLayout		mPointcloudInstanceAttribs;
	gl::GlslProgRef			mPointcloudShader;
	gl::Texture2dRef		mPointcloudTexture;

	//Particles
	Perlin					mParticlesPerlin;
	vector<Particle>		mParticlesPoints;
	gl::BatchRef			mParticlesBatch;
	gl::VboRef				mParticlesInstanceData;
	geom::BufferLayout		mParticlesInstanceAttribs;
	gl::GlslProgRef			mParticlesShader,
							mParticlesHighpassShader,
							mParticlesBlurUShader,
							mParticlesBlurVShader;

	gl::FboRef				mParticlesBaseRT,
							mParticlesHighPassRT,
							mParticlesBlurURT,
							mParticlesBlurVRT;

	gl::Texture2dRef		mParticlesTexture;

	int						mNoOfFramesBeforeSpawingParticles,
							mNoOfFramesElapsed,
							mTimeToSpawnParticles,
							mNumberOfParticlesSpawned,
							mMaxParticleCount;

	ivec3					mIterator,
							mDecrementer;

	//GUI
	params::InterfaceGlRef	mGUI;
	bool					mDrawGUI;

	int						mParamPointcloudStep;

	float					mParamPointcloudSize,
							mParamPointcloudSpecularPower,
							mParamPointcloudSpecularStrength,
							mParamPointcloudFresnelPower,
							mParamPointcloudFresnelStrength,
							mParamPointcloudReflectionStrength,
							mParamPointcloudLightPositionX,
							mParamPointcloudLightPositionY,
							mParamPointcloudLightPositionZ,
							mParamParticlesSpecularPower,
							mParamParticlesSpecularStrength,
							mParamParticlesAmbientStrength,
							mParamParticlesBlurUStep,
							mParamParticlesBlurVStep;
};
#endif