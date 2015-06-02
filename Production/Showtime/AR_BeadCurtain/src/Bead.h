#ifndef _BEAD_H_
#define _BEAD_H_
#include "cinder/Vector.h"
#include "cinder/Color.h"

using namespace ci;

class Bead
{
public:
	Bead();
	~Bead();

	void Step();
	void CheckIfGlowing(ivec2 pPoint);

	bool	IsGlowing;
	vec3	PPosition;
	Color	PColor;
	float	PSize;

private:
	int mGlowTime;
	int mGlowLife;

	vec2	mImagePos;
};
#endif