#ifndef _AR_V_
#define _AR_V_
#ifdef _DEBUG
#pragma comment(lib, "DSAPI.dbg.lib")
#else
#pragma comment(lib, "DSAPI.lib")
#endif
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Cinder-DSAPI/src/CiDSAPI.h"
#include "beadcurtain/BeadCurtain.h"
#include "displace/Displace.h"
#include "geomask/GeoMask.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AppFrame : public App
{
public:
	AppFrame() {}
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

private:
	BeadCurtainPtr	mBeadCurtain;
	DisplacePtr		mDisplace;
	GeoMaskPtr		mGeoMask;

	int				mMode;

	Channel16uRef	mDepth;
};
#endif