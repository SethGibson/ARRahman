#ifndef __CI_DSAPI__
#define __CI_DSAPI__
#include <memory>
#include "DSAPI.h"
#include "DSAPIUtil.h"
#include "cinder/Channel.h"
#include "cinder/CinderGlm.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"

using namespace ci;
using namespace std;

namespace CinderDS
{
	enum FrameSize
	{
		DEPTHSD,	// 480x360
		DEPTHVGA,	// 640x480 (628x468)
		DEPTHQVGA, // 320x240
		RGBVGA,	// 640x480
		RGBHD	// 1920x1080
	};

	enum StereoCam
	{
		DS_LEFT = DSWhichImager::DS_LEFT_IMAGER,	// this may be useful later
		DS_RIGHT = DSWhichImager::DS_RIGHT_IMAGER,
		DS_BOTH = DSWhichImager::DS_BOTH_IMAGERS
	};

	//Index, Serial Number
	typedef pair<int, uint32_t> camera_type;
#ifndef DSAPI_VER_19
	vector<camera_type> GetCameraList();
#endif
	class CinderDSAPI;
	typedef std::shared_ptr<DSAPI> DSAPIRef;
	typedef std::shared_ptr<CinderDSAPI> CinderDSRef;

	class CinderDSAPI
	{
	protected:
		CinderDSAPI();
	public:
		static CinderDSRef create();
		~CinderDSAPI();

		bool init();
#ifndef DSAPI_VER_19
		bool init(uint32_t pSerialNo);
#endif
		bool initForAlignment();
		bool initRgb(const FrameSize &pRes, const int &pFPS);
		bool initDepth(const FrameSize &pRes, const int &pFPS);
		bool initStereo(const FrameSize &pRes, const int &pFPS, const StereoCam &pWhich);
		bool start();
		bool update();
		bool stop();

		const Surface8u& getRgbFrame();
		const Channel8u& getLeftFrame();
		const Channel8u& getRightFrame();
		const Channel16u& getDepthFrame();

		const vector<ivec2>& mapDepthToColorFrame();
		//const Surface32f& mapDepthToCameraTable();

		// get a 3d point from depth image coords (image x, image y, depth)
		const vec3 getDepthSpacePoint(float pX, float pY, float pZ);
		const vec3 getDepthSpacePoint(int pX, int pY, uint16_t pZ);
		const vec3 getDepthSpacePoint(vec3 pPoint);

		//get a Color object from depth image coords (image x, image y, depth)
		const Color getColorFromDepthImage(float pX, float pY, float pZ);		
		const Color getColorFromDepthImage(int pX, int pY, uint16_t pZ);
		const Color getColorFromDepthImage(vec3 pPoint);

		//get a Color object from camera space coords (camera x, camera y, camera z)
		const Color getColorFromDepthSpace(float pX, float pY, float pZ);		
		const Color getColorFromDepthSpace(vec3 pPoint);

		//get color space UVs from depth image coords
		const vec2 getColorCoordsFromDepthImage(float pX, float pY, float pZ);

		//get color space UVs from depth camera coords
		const vec2 getColorCoordsFromDepthSpace(vec3 pPoint);

		int getDepthWidth(){ return mLRZWidth; }
		int getDepthHeight(){ return mLRZHeight; }
		const ivec2 getDepthSize(){ return ivec2(mLRZWidth, mLRZHeight); }
		const vec2 getDepthFOVs();

		int getRgbWidth(){ return mRgbWidth; }
		int getRgbHeight(){ return mRgbHeight; }
		const ivec2 getRgbSize(){ return ivec2(mRgbWidth, mRgbHeight); }
		const vec2 getRgbFOVs();

		const DSAPIRef getDSAPI();
		DSThird* getDSThird();
		const DSCalibIntrinsicsRectified getZIntrinsics();
		const DSCalibIntrinsicsRectified getRgbIntrinsics();


		
	private:
		bool	open();
		bool	setupStream(const FrameSize &pRes, ivec2 &pOutSize);

		bool	mHasValidConfig,
				mHasValidCalib,
				mHasRgb,
				mHasDepth,
				mHasLeft,
				mHasRight,
				mIsInit,
				mUpdated;

		int32_t	mLRZWidth,
				mLRZHeight,
				mRgbWidth,
				mRgbHeight;

		DSAPIRef	mDSAPI;
		DSThird		*mDSRGB;
		DSCalibIntrinsicsRectified	mZIntrinsics;
		DSCalibIntrinsicsRectified	mRgbIntrinsics;
		double						mZToRgb[3];

		Surface8u		mRgbFrame;
		Channel8u		mLeftFrame;
		Channel8u		mRightFrame;
		Channel16u		mDepthFrame;
		Surface32f		mDepthToCameraTable;

		Surface8uRef	mRgbFrameRef;
		Channel8uRef	mLeftFrameRef;
		Channel8uRef	mRightFrameRef;
		Channel16uRef	mDepthFrameRef;

		vector<ivec2> mDepthToColor;

	};
};
#endif