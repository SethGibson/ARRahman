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
static float S_GRID_SIZE = 0.5f;
static int S_FRAMES = 4;
static int S_NUM_MAX_ACTIVE = 1000;

class OrbApp : public App
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

	OrbApp(){}
	~OrbApp(){}
	void setup() override;
	void update() override;
	void draw() override;
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

void OrbApp::setup()
{
	getWindow()->setSize(960, 540);
	setFrameRate(60.0f);
	//setup shader
	mShader = gl::GlslProg::create(loadAsset("shaders/orb_render_vert.glsl"),
		loadAsset("shaders/orb_render_frag.glsl"));

	//setup positions
	for (int y = 0; y < S_GRID_DIM; ++y)
	{
		float py = lmap<float>(y, 0, S_GRID_DIM, -S_GRID_SIZE, S_GRID_SIZE);
		for (int x = 0; x < S_GRID_DIM; ++x)
		{
			float px = lmap<float>(x, 0, S_GRID_DIM, -S_GRID_SIZE, S_GRID_SIZE);
			mPoints.push_back(OniriaPoint(vec3(px, py, 0.0f),
				Color(0.0f, randFloat(0.25f, 0.85f), randFloat(0.25f, 0.85f)),
				Color(0.0f, 1.0f, 1.0f),
				randFloat(0.1f, 0.5f),
				randInt(30, 90))
				);
		}
	}
	mPositionsVbo = gl::Vbo::create(GL_ARRAY_BUFFER, mPoints, GL_DYNAMIC_DRAW);
	geom::BufferLayout posLayout;
	posLayout.append(geom::CUSTOM_0, 3, sizeof(OniriaPoint), offsetof(OniriaPoint, PPosition), 1);
	posLayout.append(geom::CUSTOM_1, 3, sizeof(OniriaPoint), offsetof(OniriaPoint, PColor), 1);
	posLayout.append(geom::CUSTOM_2, 1, sizeof(OniriaPoint), offsetof(OniriaPoint, PSize), 1);

	//setup mesh
	mMeshVbo = gl::VboMesh::create(geom::Sphere().radius(0.015f).subdivisions(4));
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


void OrbApp::update()
{
	float frames = (float)getElapsedFrames();
	updateOrb(frames);
}

void OrbApp::updateOrb(float frame)
{
	int id = 0;
	for (auto p = mPoints.begin(); p != mPoints.end(); ++p)
	{
		float normI = (float)id / (float)mPoints.size();
		float pNoise = mNoise.noise(p->PPosition.x, p->PPosition.y, p->PPosition.z);
		//nextPos.x=((cos(i+time+pNoise*0.1)/2.+pNoise*+prevPos.x*0.1)/zproj)*0.1;
		//nextPos.y=((sin(i+time+pNoise*0.2)/3.-pNoise*+prevPos.y*0.1)/zproj)*0.1;
		p->PPosition.x = p->PPosition_0.x + math<float>::cos(id + frame*0.01f);
		p->PPosition.y = p->PPosition_0.y + math<float>::sin(id + frame*0.05f);
		p->PPosition.z = math<float>::cos(id + frame*0.01f);

		id++;
	}
	mPositionsVbo->bufferData(mPoints.size()*sizeof(OniriaPoint), mPoints.data(), GL_DYNAMIC_DRAW);
}

void OrbApp::draw()
{
	gl::clear(Color(0, 0, 0));
	gl::setMatrices(mCamera);
	mPointsBatch->getGlslProg()->uniform("uLightPos", mCamera.getEyePoint());
	mPointsBatch->drawInstanced(S_GRID_DIM*S_GRID_DIM);
}

CINDER_APP(OrbApp, RendererGl(RendererGl::Options().msaa(16)))

