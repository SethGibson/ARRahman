#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

int S_NUM_PARTICLES = 1000;
class InstanceRotationApp : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;

	struct Particle
	{
		vec3	mPos;
		vec3	mVel;
		Color	mColor;
		int		mLife;
		float	mRotSpeed;

		//We'll use this to store our new per-instance transform
		mat4	mTransform;	

		Particle()
		{
			mPos = vec3(0);
			mVel = vec3(randFloat(-0.01f, 0.01f), randFloat(0.01f, 0.05f), randFloat(-0.01f, 0.01f));
			mColor = Color(randFloat(), randFloat(), randFloat());
			mRotSpeed = randFloat(-1.f, 1.f);
			mTransform = glm::mat4();
			mLife = randInt(90, 150);
		}
		
		void step(float pElapsed, vec3 pAxis)
		{
			mLife--;
			if (mLife > 0)
			{
				//set the matrix to the identitiy
				mTransform = glm::mat4();

				//first we update the position in the usual fashion...
				mPos += mVel;

				//now we can generate a model matrix from our position and rotation factors
				mTransform = glm::translate(mTransform, mPos);
				mTransform = glm::rotate(mTransform, mRotSpeed*pElapsed*0.1f, pAxis);
			}
		}
	};

private:
	vector<Particle>	mParticles;
	CameraPersp			mCamera;
	CameraUi			mCamUi;

	gl::BatchRef		mBatch;
	gl::VboRef			mData;
	geom::BufferLayout	mAttribs;
	gl::VboMeshRef		mMesh;
	gl::GlslProgRef		mShader;
};

void InstanceRotationApp::setup()
{
	getWindow()->setSize(1280, 720);
	setFrameRate(60.f);
	
	vec3 cEyePos = vec3(0, 5, 5);
	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 0.1f, 100.0f);
	mCamera.lookAt(cEyePos, vec3(0), vec3(0, 1, 0));
	mCamera.setPivotDistance(length(cEyePos));
	mCamUi = CameraUi(&mCamera, getWindow());

	mParticles.push_back(Particle());
	mShader = gl::GlslProg::create(loadAsset("vert.glsl"), loadAsset("frag.glsl"));
	mData = gl::Vbo::create(GL_ARRAY_BUFFER, mParticles, GL_DYNAMIC_DRAW);
	
	mAttribs.append(geom::CUSTOM_0, 3, sizeof(Particle), offsetof(Particle, mColor),1);
	mAttribs.append(geom::CUSTOM_1, 16, sizeof(Particle), offsetof(Particle, mTransform), 1);
	
	mMesh = gl::VboMesh::create(geom::Plane().size(vec2(0.5)).axes(vec3(1,0,0), vec3(0,1,0)));
	mMesh->appendVbo(mAttribs, mData);

	mBatch = gl::Batch::create(mMesh, mShader, { { geom::CUSTOM_0, "iColor" }, { geom::CUSTOM_1, "iModelMatrix" } });

	gl::enableAdditiveBlending();
}


void InstanceRotationApp::update()
{
	if (mParticles.size() < S_NUM_PARTICLES)
		mParticles.push_back(Particle());

	/*
	To guarantee that our particles are billboarded and rotating properly,
	we need to pass in the camera's view direction vector as our per-instance
	rotation axis. View direction is essentially the camera's Z-vector.
	*/
	for (auto pit = mParticles.begin(); pit != mParticles.end();)
	{
		pit->step((float)getElapsedFrames(), mCamera.getViewDirection());
		if (pit->mLife <= 0)
			pit = mParticles.erase(pit);
		else
			++pit;
	}

	mData->bufferData(mParticles.size()*sizeof(Particle), mParticles.data(), GL_DYNAMIC_DRAW);
}

void InstanceRotationApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatrices(mCamera);

	vec3 cRight, cUp;
	
	mCamera.getBillboardVectors(&cRight, &cUp);
	mBatch->getGlslProg()->uniform("u_CameraRight", cRight);
	mBatch->getGlslProg()->uniform("u_CameraUp", cUp);
	mBatch->drawInstanced(mParticles.size());
}

CINDER_APP( InstanceRotationApp, RendererGl )
