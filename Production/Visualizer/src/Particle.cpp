#include "Particle.h"	
Particle::~Particle()
{
}

Particle::Particle(vec3 pPos)
{
	PPosition = pPos;// +Rand::randVec3f() * Rand::randFloat(5.0f);
	PPosition.y -= 750;

	mRotSpeed = randFloat(-1.f, 1.f);
	mVelocity = vec3(randFloat(-5.0f, 5.0f), randFloat(0.2f, 0.7f), randFloat(-5.0f, 5.0f));

	Age = Rand::randInt(1500, 2000);
	PSize = randFloat(3.5f, 8.5f);
	mLifeSpan = Age;
	PModelMatrix = mat4();
	PModelMatrix = glm::rotate(PModelMatrix, randFloat(-0.5f, 0.5f), vec3(0, 0, 1));
	PModelMatrix = glm::rotate(PModelMatrix, randFloat(-0.5f, 0.5f), vec3(1, 0, 0));
	PModelMatrix = glm::rotate(PModelMatrix, randFloat(-1, 1), vec3(0, 1, 0));

}

Particle::Particle(vec3 pPosition, vec2 pUV, float pSize) : PPosition(pPosition), PUV(pUV), PSize(pSize)
{

}

void Particle::Step(float pElapsed, const Perlin &pNoise)
{
	Age--;
	if (Age>0)
	{
		PAlpha = math<float>::min(1, ((float)Age * 0.5f * (float)Age) / (float)mLifeSpan);

		PPosition.y += mVelocity.y;
		float cNoise = pNoise.fBm(vec3(PPosition.x*0.005f, PPosition.y*0.01f, PPosition.z*0.005f));
		float cAngle = cNoise*15.0f;
		PPosition.x += (cos(cAngle)*mVelocity.x*(1.0f-PAlpha))*0.1f;
		PPosition.z += (sin(cAngle)*mVelocity.z*(1.0f - PAlpha))*0.1f;
		mVelocity.x *= 1.001f;
		mVelocity.y *= .99999f;
		mVelocity.z *= 1.001f;

		float elapsedFrames = pElapsed;
		//PModelMatrix = mat4();
		//PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.2f, vec3(0, 0, 1));
		//PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.2f, vec3(1, 0, 0));
		//PModelMatrix = glm::rotate(PModelMatrix, mRotSpeed *elapsedFrames* 0.2f, vec3(0, 1, 0));
		//PModelMatrix = glm::translate(PModelMatrix, PPosition);



	}
}
