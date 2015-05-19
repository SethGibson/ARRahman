#include "Particle.h"	
Particle::~Particle()
{
}

Particle::Particle(vec3 pPos)
{
	PPosition = pPos + Rand::randVec3f() * Rand::randFloat(5.0f);

	mRotSpeed = randFloat(-1.f, 1.f);
	mVelocity = vec3(randFloat(-5.0f, 5.0f), randFloat(0.1f, 1.0f), randFloat(-5.0f, 5.0f));

	Age = Rand::randInt(90, 450);
	PSize = randFloat(1.5f, 5.5f);
	mLifeSpan = Age;
}

Particle::Particle(vec3 pPosition, vec2 pUV, float pSize) : PPosition(pPosition), PUV(pUV), PSize(pSize)
{

}

void Particle::Step(float pElapsed, const Perlin &pNoise)
{
	Age--;
	if (Age>0)
	{
		PAlpha = (float)Age / (float)mLifeSpan;

		PPosition.y += mVelocity.y;
		float cNoise = pNoise.fBm(vec3(PPosition.x*0.005f, PPosition.y*0.01f, PPosition.z*0.005f));
		float cAngle = cNoise*15.0f;
		PPosition.x += (cos(cAngle)*mVelocity.x*(1.0f-PAlpha))*0.1f;
		PPosition.z += (sin(cAngle)*mVelocity.z*(1.0f - PAlpha))*0.1f;
		mVelocity.x *= 1.001f;
		mVelocity.y *= .99f;
		mVelocity.z *= 1.001f;

		float elapsedFrames = pElapsed;
		PModelMatrix = mat4();
		PModelMatrix = glm::translate(PModelMatrix, PPosition);
		//PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.2f, vec3(1, 0, 0));
		//PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.2f, vec3(0, 1, 0));
		PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.2f, vec3(0, 0, 1));


	}
}
