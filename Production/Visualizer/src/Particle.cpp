#include "Particle.h"	
Particle::~Particle()
{
}

Particle::Particle(vec3 pPos)
{
	PPosition = pPos + Rand::randVec3f() * Rand::randFloat(5.0f);

	mRotSpeed = randFloat(-1.f, 1.f);
	mVelocity = Rand::randFloat(0.007f, 0.08f);  /// change the gravity here
	mDirection = vec3(Rand::randFloat(-3.0f, 3.0f), -3.0f, Rand::randFloat(0.3f, 0.6f)); // change the direction of the particles.

	Age = Rand::randInt(450, 550);
	PSize = randFloat(0.5f, 2.5f);
}

void Particle::Step(float pElapsed, const Perlin &pNoise, const vec3 &pRightDir)
{
	Age--;
	if (Age>0)
	{
		float nX = PPosition.x * 0.005f;
		float nY = PPosition.y * 0.005f;
		float nZ = pElapsed * 0.1f;
		float noise = pNoise.fBm(nX, nY, nZ) * 0.2;

		float elapsedFrames = pElapsed;

		//update location and add perlin noise
		PPosition += mDirection*mVelocity + noise;

		PModelMatrix = mat4();
		PModelMatrix = glm::translate(PModelMatrix, PPosition);

		PModelMatrix = glm::rotate(PModelMatrix, 90.0f, pRightDir);
		PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.2f, vec3(0, 0, 1));

		//PModelMatrix = glm::translate(PModelMatrix, vec3(0));
		PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.2f, vec3(0, 1, 0));
		PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.2f, vec3(1, 0, 0));
	}
}
