#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static int S_GRID_DIM = 128;
static float S_GRID_SIZE = 3.0f;
static int S_FRAMES = 4;
static int S_NUM_MAX_ACTIVE = 1000;

class OniriaApp : public App
{
public:
	struct OniriaPoint
	{
	public:
		vec3		PPosition,
					PPosition_0;

		Color		PColorBase,
					PColorHilite,
					PColor;

		float		PSize;

		int			PAge, PLife;
		bool		PActive;

		OniriaPoint(){}
		OniriaPoint(vec3 pPos, Color pColA, Color pColB, float pSize, int pAge) :
			PPosition(pPos), PPosition_0(pPos), PColorBase(pColA), PColorHilite(pColB),
			PColor(pColA), PSize(pSize), PActive(false), PLife(pAge)
		{}
	};

	OniriaApp(){}
	~OniriaApp(){}
	void setup() override;
	void update() override;
	void draw() override;
	void updateWaves(float frames);
	void updateOrb(float frames);

private:
	gl::BatchRef		mPointsBatch;
	gl::VboMeshRef		mMeshVbo;
	gl::VboRef			mPositionsVbo;
	vector<OniriaPoint>	mPoints;
	gl::GlslProgRef		mShader;

	CameraPersp			mCamera;
	CameraUi			mCamUi;

	Perlin				mNoise;

	int					mStartTime;
	int					mNumActive;
};

void OniriaApp::setup()
{
	getWindow()->setSize(960, 540);
	setFrameRate(60.0f);
	//setup shader
	mShader = gl::GlslProg::create(loadAsset("shaders/oniria_circles_vert.glsl"),
									loadAsset("shaders/oniria_circles_frag.glsl"));

	//setup positions
	for (int y = 0; y < S_GRID_DIM; ++y)
	{
		float py = lmap<float>(y, 0, S_GRID_DIM, -S_GRID_SIZE, S_GRID_SIZE);
		for (int x = 0; x < S_GRID_DIM; ++x)
		{
			float px = lmap<float>(x, 0, S_GRID_DIM, -S_GRID_SIZE, S_GRID_SIZE);
			mPoints.push_back(OniriaPoint(vec3(px, py, 0.0f),
											Color(0.0f, randFloat(0.25f, 0.85f), randFloat(0.25f, 0.85f)),
											Color(0.0f,1.0f,1.0f),
											randFloat(0.1f,0.5f),
											randInt(30,90))
								);
		}
	}
	mPositionsVbo = gl::Vbo::create(GL_ARRAY_BUFFER, mPoints, GL_DYNAMIC_DRAW);
	geom::BufferLayout posLayout;
	posLayout.append(geom::CUSTOM_0, 3, sizeof(OniriaPoint), offsetof(OniriaPoint,PPosition), 1);
	posLayout.append(geom::CUSTOM_1, 3, sizeof(OniriaPoint), offsetof(OniriaPoint, PColor), 1);
	posLayout.append(geom::CUSTOM_2, 1, sizeof(OniriaPoint), offsetof(OniriaPoint, PSize), 1);

	//setup mesh
	mMeshVbo = gl::VboMesh::create(geom::Sphere().radius(0.015f).subdivisions(16));
	mMeshVbo->appendVbo(posLayout, mPositionsVbo);

	//setup batch
	mPointsBatch = gl::Batch::create(mMeshVbo, mShader, { { geom::CUSTOM_0, "iPosition" },
															{ geom::CUSTOM_1, "iColor" },
															{ geom::CUSTOM_2, "iSize" } });

	mCamera.setPerspective(45.0f, getWindowAspectRatio(), 0.1f, 100.0f);
	mCamera.lookAt(vec3(0, 0, 2.f), vec3(0), vec3(0, 1, 0));
	mCamera.setPivotDistance(2.0f);
	mCamUi = CameraUi(&mCamera, getWindow());

	mNoise = Perlin();
	mStartTime = getElapsedFrames();
	mNumActive = 0;
}


void OniriaApp::update()
{
	float frames = (float)getElapsedFrames();
	updateWaves(frames);
}

void OniriaApp::updateWaves(float frames)
{
	int id = 0;
	int activeId = randInt(0, mPoints.size());

	for (auto p = mPoints.begin(); p != mPoints.end(); ++p)
	{
		int xId = id % S_GRID_DIM;
		int yId = id / S_GRID_DIM;
		int columnStep = xId % 2;
		int rowStep = yId % 2;

		vec3 position(p->PPosition.x, p->PPosition.y, p->PPosition.z + frames*0.01f);
		float noise3 = mNoise.noise(position);

		float xOffset = 0.08f*(
			(math<float>::cos(frames*0.02f)*columnStep) +
			(math<float>::sin(frames*0.02f)*1.0 - columnStep)
			) + noise3*0.4f;


		float yOffset = 0.08f*(
			(math<float>::sin(frames*0.02f)*rowStep) +
			(math<float>::cos(frames*0.02f)*1.0 - rowStep)
			) + noise3*0.18f;


		p->PPosition.x = p->PPosition_0.x + xOffset;
		p->PPosition.y = p->PPosition_0.y + xOffset;
		p->PPosition.z = (
			(math<float>::sin(p->PPosition.x + frames*0.02f)*noise3*0.1f)
			+ (math<float>::cos(p->PPosition.y + frames*0.01f)*noise3*0.1f)
			)*0.025f + noise3*0.25f;

		if (id == activeId)
		{
			if (mNumActive < S_NUM_MAX_ACTIVE)
			{
				if (p->PActive == false)
				{
					p->PActive = true;
					mNumActive++;
				}
			}
		}


		if (p->PActive)
			p->PSize = 1.0f;
		else
			p->PSize = (math<float>::sin(xId + yId + frames*0.1) + math<float>::cos(xId + yId + frames*0.1))*0.5f;




		id++;
	}

	mPositionsVbo->bufferData(mPoints.size()*sizeof(OniriaPoint), mPoints.data(), GL_DYNAMIC_DRAW);

}

void OniriaApp::updateOrb(float frame)
{
	int id = 0;

	/*
	for (auto p = mPoints.begin(); p != mPoints.end(); ++p)
	{
		float normI = (float)id / (float)mPoints.size();
		float pNoise = mNoise.noise(p->PPosition);
		//nextPos.x=((cos(i+time+pNoise*0.1)/2.+pNoise*+prevPos.x*0.1)/zproj)*0.1;
		//nextPos.y=((sin(i+time+pNoise*0.2)/3.-pNoise*+prevPos.y*0.1)/zproj)*0.1;
		p->PPosition.x = (normI*math<float>::cos((id + frame*.02f))) + pNoise*0.5f;
		//nextPos.z = 0.0;
		p->PPosition.y = normI*math<float>::sin((id + frame*.01f)) + pNoise*0.1f;
		p->PPosition.z = (normI*math<float>::cos((id + frame*.02f))) + pNoise*0.04f;
		//nextPos.w = 1. - pow((pos.x*pos.x), 0.25);
		id++;
	}*/
	for (auto p = mPoints.begin(); p != mPoints.end(); ++p)
	{
		float normI = (float)id / (float)mPoints.size();
		float pNoise = mNoise.noise(p->PPosition.x, p->PPosition.y, p->PPosition.z+frame*0.01f);
		//nextPos.x=((cos(i+time+pNoise*0.1)/2.+pNoise*+prevPos.x*0.1)/zproj)*0.1;
		//nextPos.y=((sin(i+time+pNoise*0.2)/3.-pNoise*+prevPos.y*0.1)/zproj)*0.1;
		p->PPosition.x = normI*math<float>::cos(id + frame*0.01f)+pNoise;
		p->PPosition.y = normI*math<float>::sin(id + frame*0.005f) + pNoise;
		p->PPosition.z = math<float>::cos(id + frame*0.01f) + pNoise*normI;

		id++;
	}
	mPositionsVbo->bufferData(mPoints.size()*sizeof(OniriaPoint), mPoints.data(), GL_DYNAMIC_DRAW);
}

void OniriaApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatrices(mCamera);
	mPointsBatch->getGlslProg()->uniform("uLightPos", mCamera.getEyePoint());
	mPointsBatch->drawInstanced(S_GRID_DIM*S_GRID_DIM);
}

CINDER_APP( OniriaApp, RendererGl( RendererGl::Options().msaa(16) ))
