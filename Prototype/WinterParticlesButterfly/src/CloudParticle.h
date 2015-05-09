#ifndef _CLOUD_PARTICLE_H_
#define _CLOUD_PARTICLE_H_
#include "cinder/CinderGlm.h"
#include "cinder/Matrix.h"
#include "cinder/Perlin.h"
#include "cinder/Vector.h"
using namespace ci;

struct CloudPoint
{
	vec3 PPosition;
	vec2 PUV;
	CloudPoint(vec3 pPos, vec2 pUV) : PPosition(pPos), PUV(pUV){}
};

class CloudParticle
{
public:
	CloudParticle(){}
	CloudParticle(vec3 pPos)
	{
		PPosition = pPos + Rand::randVec3f() * Rand::randFloat(5.0f);

		mRotSpeed = randFloat(-1.f, 1.f);
		mVelocity = Rand::randFloat(0.08f, 0.15f);  /// change the gravity here
		mDirection = vec3(Rand::randFloat(-3.0f, 3.0f), -3.0f, Rand::randFloat(0.3f, 0.6f)); // change the direction of the particles.
		mRadius = Rand::randFloat(1.0f, 3.0f);

		mLifespan = Rand::randInt(450, 550);
		Age = mLifespan;
		PSize = randFloat(0.5f, 2.5f);
	}

	void step(float pElapsed, const Perlin &pNoise, const vec3 &pRightDir)
	{
		Age--;
		if (Age>0)
		{
			float nX = PPosition.x * 0.005f;
			float nY = PPosition.y * 0.005f;
			float nZ = pElapsed * 0.1f;
			float noise = pNoise.fBm(nX, nY, nZ) * 0.7;

			float elapsedFrames = pElapsed;

			//update location and add perlin noise
			PPosition += mDirection*mVelocity + noise;

			PModelMatrix = mat4();
			PModelMatrix = glm::translate(PModelMatrix, PPosition);
			PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.1f, vec3(0.5, 0.5, 0.5));
			PModelMatrix = glm::rotate(PModelMatrix, 90.0f, pRightDir);
		}
	}

	mat4	PModelMatrix;
	vec3	PPosition;
	int		Age;
	float	PSize;

private:
	vec3	mAcceleration,
			mDirection;

	float	mDecay,
			mRadius, mRadiusDest,
			mScale,
			mVelocity,
			mRotSpeed,
			mMass;

	int		mLifespan;
};

#endif _CLOUD_PARTICLE_H_