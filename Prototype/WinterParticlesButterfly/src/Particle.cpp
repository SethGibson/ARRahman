#include "Particle.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"

using namespace ci;

Particle::Particle()
{

}

Particle::Particle(vec3 loc, vec3 vel)
{
	
		mTransform = glm::mat4();
		mLoc = loc + Rand::randVec3f() * Rand::randFloat(5.0f);
		vec3 velOffset = Rand::randVec3f() * Rand::randFloat(1.0f, 3.0f);
		//mVel = vel * 5.0f + velOffset;
		//mAcc = vec3(0);

		//mDecay = Rand::randFloat(0.95f, 0.951f);
		//mRadius = 3.0f + Rand::randFloat(0.1);
		//mRadiusDest = 3.0f;
		//mMass = mRadius * mRadius * 0.0001f + 0.01f;
		//mScale = 3.0f;
		mRotSpeed = randFloat(-1.f, 1.f);
		mVelocity = Rand::randFloat(0.08f, 0.15f);  /// change the gravity here
		mDir = vec3(Rand::randFloat(-3.0f, 3.0f), -3.0f, Rand::randFloat(0.3f, 0.6f)); // change the direction of the particles.
		mRadius = Rand::randFloat(1.0f, 3.0f);

		

		mAge = 0;
		mLifespan = Rand::randInt(450, 550);
		mIsDead = false;
	

}

Particle::Particle(vec3 loc)
{
	mLoc = loc;
	//mVel = vel;
	mVelocity = Rand::randFloat(0.3f, 0.6f);
	mDir = vec3(Rand::randFloat(0.3f, 0.6f), 3.0f, Rand::randFloat(0.3f, 0.6f));

	//mAcc = vec3(0);

	//mDecay = Rand::randFloat(0.95f, 0.951f);
	//mRadius = 3.0f + Rand::randFloat(0.1);
	mRadius = Rand::randFloat(1.0f, 3.0f);
	//mRadiusDest = 3.0f;
	//mMass = mRadius * mRadius * 0.0001f + 0.01f;
	//mScale = 3.0f;
	mAge = 0;
	mLifespan = Rand::randInt(50, 250);
	mIsDead = false;
}



//void Particle::applyPerlin(const Perlin &perlin)
//{
//	//still only for 2D needs to be modded for 3D
//	float nX = mLoc.x * 0.005f;
//	float nY = mLoc.y * 0.005f;
//	float nZ = app::getElapsedFrames() * 0.0025f;
//	float noise = perlin.fBm(nX, nY, nZ);
//	float angle = noise * 15.0f;
//	vec2 noiseVector(cos(angle), sin(angle));
//	mVel += noiseVector * mMass * 5.0f;
//}

//void Particle::update(const Channel32f &channel, const ivec2 &mouseLoc)
//{
//	mVel += mAcc;
//
//	float maxVel = mRadius + 0.0025f;
//
//	float velLength = mVel.lengthSquared() + 0.1f;
//	if (velLength > maxVel*maxVel){
//		mVel.normalize();
//		mVel *= maxVel;
//	}
//
//	mLoc += mVel;
//	mVel *= mDecay;
//	mAcc.set(0, 0);
//
//	if (mLoc.x >= 0.0f && mLoc.x <= (float)app::getWindowWidth() - 1 &&
//		mLoc.y >= 0.0f && mLoc.y <= (float)app::getWindowHeight() - 1){
//
//		mRadiusDest = channel.getValue(mLoc) * mScale + 0.5f;
//	}
//	else {
//		mRadiusDest = 0.1f;
//	}
//
//	mRadius -= (mRadius - mRadiusDest) * 0.1f;
//	mMass = mRadius * mRadius * 0.0001f + 0.01f;
//
//	//Uncomment these lines to enable particles to die over time
//	//mAge++;
//	//if( mAge > mLifespan ) mIsDead = true;
//}




void Particle::update()
{
	//Update particle position and kill them here.

	mLoc += mDir * mVelocity;

	//Uncomment these lines to enable particles to die over time
	mAge++;
	if( mAge > mLifespan ) mIsDead = true;
}

void Particle::draw()
{
	gl::drawSphere(mLoc, mRadius, 16);
	
}



