#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

static int S_MAX_RINGS = 5;

class AR_DisplaceApp : public App
{
public:
	void setup() override;
	void update() override;
	void draw() override;

	struct Ringu
	{
		int		PRadius, PRotRadius;
		vec2	PPosition, PCenter;
		int		PAge, PLife;
		float	PRotSpeed, PGrowthRate;
		ColorA	PColor;
		int		PWidth;

		Ringu(vec2 pWindowCenter)
		{
			PRadius = randInt(160, 240);
			PRotRadius = randInt(20, 60);
			PCenter = vec2(randFloat(5, 10), randFloat(5, 10)) + pWindowCenter;
			PPosition = PCenter;
			PLife = randInt(150, 300);
			PAge = PLife;
			PRotSpeed = randFloat(-0.05f, 0.05f);
			PGrowthRate = randFloat(0.35f, 1.0f);
			PColor = ColorA(randFloat(0.1f, 1.0f), 0.0, randFloat(0.1f, 1.0f),1.0f);
			PWidth = randInt(1, 4);
		}

		void Step(int pFrames)
		{
			PAge--;
			float normAge = (float)PAge / (float)PLife;
			PColor.a = normAge;
			PRadius += PGrowthRate;
			PPosition.x = PRotRadius*math<float>::cos(pFrames*PRotSpeed) + PCenter.x;
			PPosition.y = PRotRadius*math<float>::sin(pFrames*PRotSpeed) + PCenter.y;
		}
	};

private:
	vector<Ringu>	mRings;
};

void AR_DisplaceApp::setup()
{
	getWindow()->setSize(960, 540);
	setFrameRate(60.0f);
	gl::enableAlphaBlending();
}

void AR_DisplaceApp::update()
{
	if (mRings.size() < S_MAX_RINGS)
		mRings.push_back(Ringu(getWindowCenter()));

	for (auto r = mRings.begin(); r != mRings.end();)
	{
		r->Step(getElapsedFrames());
		if (r->PAge <= 0)
			r = mRings.erase(r);
		else
			++r;
	}


}

void AR_DisplaceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatricesWindow(getWindowSize()); 

	for (auto r : mRings)
	{
		gl::color(r.PColor);
		gl::lineWidth(r.PWidth);
		gl::drawStrokedCircle(r.PPosition, r.PRadius);
	}
}

CINDER_APP( AR_DisplaceApp, RendererGl )
