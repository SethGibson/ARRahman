#ifndef _DISPLACE_
#define _DISPLACE_
#include <memory>
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/gl.h"

using namespace std;
using namespace ci;
class AppFrame;

class Displace;
typedef unique_ptr<Displace> DisplacePtr;

struct Ringu
{
	Ringu();
	void Step(float elapsedFrames);

	vec3				Position, Center;
	float				InnerRadius,
						OuterRadius,
						RotationSpeed;

	int					Age, Life;
	ColorA				RingColor;

	gl::VboMeshRef		RingShape;
};

class Displace
{
public:
	static DisplacePtr create() { return DisplacePtr(new Displace()); }
	~Displace();

	void Init(AppFrame *pParent);
	void Update(Channel16uRef pDepthChannel);
	void Draw();

protected:
	Displace();

private:
	void setupRings();

	void updateRings();
	void updateDepth(Channel16uRef pDepthChannel);
	void updateFbo();

	AppFrame		*mParent;
	vector<Ringu>	mRings;
	CameraPersp		mCamera;

	gl::Texture2dRef		mTexDepth;

	gl::GlslProgRef	mShaderRing,
					mShaderBlur,
					mShaderDepth;

	gl::FboRef		mFboRaw,
					mFboBlurH,
					mFboBlurV,
					mFboDepth;

	int				mTimer,
					mSpawnTime;

	float			mElapsedFrames;
};

#endif