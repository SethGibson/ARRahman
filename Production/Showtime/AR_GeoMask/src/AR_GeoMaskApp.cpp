#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//DEBUG
static int S_NUM_HEDRONS = 100;
static int S_SPAWN_COUNT = 20;
static int S_SPAWN_TIME = 30;
class AR_GeoMaskApp : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;

	struct HParticle
	{
		mat4	PModelMatrix;
		vec3	PPosition;
		vec3	PRotation;
		float	PSize;
		float	PSpeed, PAlpha;
		int		PAge;

		HParticle(){}
		HParticle(vec3 pPos) :PPosition(pPos)
		{
			PSize = randFloat(0.5f, 2.0f);
			PSpeed = randFloat(0.025f, 0.1f);
			PModelMatrix = mat4();
			PModelMatrix = glm::translate(PModelMatrix, PPosition);
			PRotation = vec3(randFloat(0.001f, 0.01f), randFloat(0.001f, 0.01f), randFloat(0.001f, 0.01f));
			PAlpha = 0.0f;
		}

		void Step(float pFrames)
		{
			PPosition.z -= PSpeed;
			if (PPosition.z >= 15.0f)
			{
				float nPos = 20.0f - PPosition.z;
				PAlpha = lmap<float>(nPos, 5.0f, 0.0f, 1.0f, 0.0f);
			}
			else if (PPosition.z <= -15.0f)
			{
				float nPos = PPosition.z + 20.0f;
				PAlpha = lmap<float>(nPos, 5.0f, 0.0f, 1.0f, 0.0f);
			}
			
			PModelMatrix = mat4();
			PModelMatrix *= glm::translate(PModelMatrix, PPosition);

			auto rot = quat(PRotation*pFrames);
			PModelMatrix *= glm::toMat4(rot);
			//glm::rot
		}
	};

private:
	gl::GlslProgRef		mShader;
	gl::VboMeshRef		mMesh;
	vector<HParticle>	mParticles;
	gl::VboRef			mInstances;
	gl::Texture2dRef	mTexBg;
	gl::BatchRef		mBatch;

	CameraPersp			mCamera;
	CameraUi			mCameraUi;

	int					mTimer;
};

void AR_GeoMaskApp::setup()
{
	getWindow()->setSize(960, 540);
	setFrameRate(60.0f);
	mTexBg = gl::Texture::create(loadImage(loadAsset("textures/test_bg.png")));
	
	float pX = randFloat(-10.0f, 10.0f);
	float pY = randFloat(-10.0f, 10.0f);
	mParticles.push_back(HParticle(vec3(pX,pY,10.0f)));

	mShader = gl::GlslProg::create(loadAsset("shaders/particle_vert.glsl"), loadAsset("shaders/particle_frag.glsl"));
	mMesh = gl::VboMesh::create(geom::Icosahedron());
	mInstances = gl::Vbo::create(GL_ARRAY_BUFFER, mParticles, GL_DYNAMIC_DRAW);
	geom::BufferLayout attribs;
	attribs.append(geom::CUSTOM_0, 1, sizeof(HParticle), offsetof(HParticle, PSize), 1);
	attribs.append(geom::CUSTOM_1, 16, sizeof(HParticle), offsetof(HParticle, PModelMatrix), 1);
	attribs.append(geom::CUSTOM_2, 1, sizeof(HParticle), offsetof(HParticle, PAlpha), 1);

	mMesh->appendVbo(attribs, mInstances);
	mBatch = gl::Batch::create(mMesh, mShader, { { geom::CUSTOM_0, "iSize" },
													{ geom::CUSTOM_1, "iModelMatrix" },
													{ geom::CUSTOM_2, "iAlpha" } });

	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 0.1f, 100.0f);
	mCamera.lookAt(vec3(0, 0, -19), vec3(0), vec3(0, 1, 0));
	mCamera.setPivotDistance(19);
	mCameraUi = CameraUi(&mCamera, getWindow());

	gl::enableDepthWrite();

	mTimer = getElapsedFrames();
}

void AR_GeoMaskApp::update()
{
	if (mParticles.size() < S_NUM_HEDRONS)
	{
		if (getElapsedFrames() - mTimer > S_SPAWN_TIME)
		{
			mTimer = getElapsedFrames();
			float pX = randFloat(-10.0f, 10.0f);
			float pY = randFloat(-10.0f, 10.0f);
			mParticles.push_back(HParticle(vec3(pX, pY, 20.0f)));
		}
	}

	for (auto p = mParticles.begin(); p != mParticles.end();)
	{
		p->Step(static_cast<float>(getElapsedFrames()));
		if (p->PPosition.z <= -20.0f)
			p = mParticles.erase(p);
		else
			++p;
	}

	mInstances->bufferData(sizeof(HParticle)*mParticles.size(), mParticles.data(), GL_DYNAMIC_DRAW);

	float cameraX = -15.0f*math<float>::cos((M_PI / 2) + getElapsedFrames()*0.001f);
	float cameraZ = -15.0f*math<float>::sin((M_PI / 2)+getElapsedFrames()*0.001f);
	
	mCamera.lookAt(vec3(cameraX, 0.0f, cameraZ), vec3(0), vec3(0, 1, 0));
}

void AR_GeoMaskApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
	
	gl::disableDepthRead();
	gl::setMatricesWindow(getWindowSize());
	gl::draw(mTexBg, Rectf({ vec2(0), getWindowSize() }));
	
	gl::enableDepthRead();
	gl::enableAlphaBlending();
	gl::setMatrices(mCamera);
	mBatch->drawInstanced(S_NUM_HEDRONS);
	gl::disableAlphaBlending();
}

CINDER_APP( AR_GeoMaskApp, RendererGl(RendererGl::Options().msaa(16)) )
