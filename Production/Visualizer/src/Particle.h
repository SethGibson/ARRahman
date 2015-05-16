#ifndef _PARTICLE_H_
#define _PARTICLE_H_
#include "cinder/CinderGlm.h"
#include "cinder/Matrix.h"
#include "cinder/Perlin.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"

using namespace ci;
class Particle
{
public:
	Particle();
	Particle(vec3 pPosition);
	Particle(vec3 pPosition, vec2 pUV);
	~Particle();

	void Step(float pElapsed, const Perlin &pNoise, const vec3 &pRightDir);

	mat4	PModelMatrix;
	vec3	PPosition;
	vec2	PUV;
	int		Age;
	float	PSize;

private:
	vec3	mAcceleration,
			mDirection;

	float	mRotSpeed,
			mVelocity;
};

#endif