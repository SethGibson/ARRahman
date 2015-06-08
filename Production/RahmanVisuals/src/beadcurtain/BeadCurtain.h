#ifndef _BEAD_CURTAIN_
#define _BEAD_CURTAIN_
#include <memory>
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

using namespace std;
using namespace ci;
class AppFrame;

struct Bead
{
	vec3	Position;
	vec2	Position2d;
	float	Radius;
	bool	Active;
	int		Life;
	int		Age;
	Color	EmissiveColor;
	Color	ActiveColor;

	Bead();
	Bead(vec3 pPos, vec2 pIPos);
	void Step(float frames);
};

class BeadCurtain;
typedef unique_ptr<BeadCurtain> BeadCurtainPtr;

class BeadCurtain
{
public:
	static BeadCurtainPtr create() { return BeadCurtainPtr(new BeadCurtain()); }
	~BeadCurtain();

	void Init(AppFrame *pParent);
	void Update(Channel16uRef pDepthChannel);
	void Draw();

protected:
	BeadCurtain();

private:
	void setupBeads();
	void setupFbo();
	void updateBeads(Channel16uRef pDepthChannel);
	void updateFbo();

	AppFrame				*mParent;
	gl::VboMeshRef			mBeadMesh;
	gl::VboRef				mPositions;
	gl::BatchRef			mCurtain;
	gl::TextureCubeMapRef	mEnvMap;

	gl::FboRef				mFboRaw,
							mFboHiPass,
							mFboBlurU,
							mFboBlurV;

	gl::GlslProgRef			mShaderBead,
							mShaderHiPass,
							mShaderBlur;

	ivec2					mWindowSize;
	CameraPersp				mCamera;

	vector<Bead>			mBeads;

	float					mElapsedFrames;
	
};
#endif