#ifndef _PARTICLE_H_
#define _PARTICLE_H_
#include "cinder/CinderGlm.h"
#include "cinder/Matrix.h"
#include "cinder/Perlin.h"
#include "cinder/Vector.h"
#include "cinder/Rand.h"

using namespace ci;

// TODO: Consider creating derived classes
class Particle
{
public:
	Particle();
	Particle(vec3 pPosition);							//Generic Particle Constructor
	Particle(vec3 pPosition, vec2 pUV, float pSize);	//Pointcloud Particle Constructor
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