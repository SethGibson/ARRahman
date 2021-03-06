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
#include "cinder/qtime/QuickTimeGl.h"
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
	void setupPointCloud(pair<string, string> pShaders, vector<pair<string,int>> pSamplerUniforms);
	void setupParticles(string pObjFile, string pTextureFile, pair<string, string> pShaders, vector<pair<string, int>> pSamplerUniforms);
	void setupFBOs();

	void updatePointCloud();
	void updateParticles();
	void updateFBOs();

	void drawPointCloud();
	void drawParticles();
	void drawFBO();

	CinderDSRef		mDS;

	//Scene
	CameraPersp				mCamera;
	CameraUi				mCameraUi;
	
	//Skybox
	gl::TextureRef			mBackgroundTexture;
	qtime::MovieGlRef		mBackgroundPlayer;
	vec2					mBackgroundSize;

	//Point Cloud
	vector<Particle>		mPointcloudPoints;
	gl::BatchRef			mPointcloudBatch;
	gl::VboRef				mPointcloudInstanceData;
	geom::BufferLayout		mPointcloudInstanceAttribs;
	gl::GlslProgRef			mPointcloudShader;
	gl::Texture2dRef		mPointcloudTexture;

	//Particles
	Perlin							mParticlesPerlin;
	vector<Particle>				mParticlesPoints;
	gl::BatchRef					mParticlesBatch;
	gl::VboRef						mParticlesInstanceData;
	geom::BufferLayout				mParticlesInstanceAttribs;
	gl::GlslProgRef					mParticlesShader,
									mParticlesBlurShader;

	gl::FboRef						mParticlesBaseRT,
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

	float					mParamPointcloudMinDepth,
							mParamPointcloudMaxDepth,
							mParamPointcloudSize,
							mParamPointcloudTextureMinR,
							mParamPointcloudTextureMinG,
							mParamPointcloudTextureMinB,
							mParamPointcloudSpecularPower,
							mParamPointcloudSpecularStrength,
							mParamPointcloudFresnelPower,
							mParamPointcloudFresnelStrength,
							mParamPointcloudAmbientStrength,
							mParamPointcloudReflectionStrength,
							mParamPointcloudLightPositionX,
							mParamPointcloudLightPositionY,
							mParamPointcloudLightPositionZ,
							mParamParticlesSpecularPower,
							mParamParticlesSpecularStrength,
							mParamParticlesAmbientStrength,
							mParamParticlesBlurSizeU,
							mParamParticlesBlurSizeV,
							mParamParticlesBlurStrength;
};
#endif