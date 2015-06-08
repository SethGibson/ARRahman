#include "RahmanVisualsApp.h"

void AppFrame::setup()
{
	mBeadCurtain = BeadCurtain::create();
	mDisplace = Displace::create();
	mGeoMask = GeoMask::create();

	mBeadCurtain->Init(this);
	mDisplace->Init(this);
}

void AppFrame::mouseDown( MouseEvent event )
{
}

void AppFrame::update()
{
	switch (mMode)
	{
	case 0:
		mBeadCurtain->Update(mDepth);
		break;
	case 1:
		mDisplace->Update(mDepth);
		break;
	case 2:
		mGeoMask->Update();
	}
}

void AppFrame::draw()
{
	switch (mMode)
	{
	case 0:
		mBeadCurtain->Draw();
		break;
	case 1:
		mDisplace->Draw();
		break;
	case 2:
		mGeoMask->Draw();
	}
}

CINDER_APP( AppFrame, RendererGl )
