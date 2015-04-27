#pragma once
#include "cinder/Vector.h"
#include "cinder/Color.h"
#include "cinder/Channel.h"
#include "cinder/Perlin.h"
#include <vector>

class Particle {
 public:
	//Particle();
	//Particle( ci::Vec3f pos, ci::Vec3f vel );
	//void pullToCenter( const ci::Vec3f &center );
	//void update( bool flatten );
	//void draw();
	//void drawTail();
	//void limitSpeed();
	//void addNeighborPos( ci::Vec3f pos );

	//ci::Vec3f	mPos;
	//ci::Vec3f	mTailPos;
	//ci::Vec3f	mVel;
	//ci::Vec3f	mVelNormal;
	//ci::Vec3f	mAcc;
	//
	//ci::Color	mColor;
	//
	//ci::Vec3f	mNeighborPos;
	//int			mNumNeighbors;
	//
	//float		mDecay;
	//float		mRadius;
	//float		mLength;
	//float		mMaxSpeed, mMaxSpeedSqrd;
	//float		mMinSpeed, mMinSpeedSqrd;
	//float		mCrowdFactor;

	 Particle();
	 Particle(ci::vec3);
	 Particle(ci::vec3 loc, ci::vec3 vel);

	// void applyPerlin(const ci::Perlin &perlin);
	// void update(const ci::Channel32f &channel, const ci::ivec2 &mouseLoc);
	 void update();
	 void draw();

	 ci::vec3	mLoc;
	 ci::vec3	mVel;
	 ci::vec3	mAcc;
	 ci::vec3   mDir;

	 float		mDecay;
	 float		mRadius, mRadiusDest;
	 float		mScale;
	 float		mVelocity;

	 float		mMass;

	 int		mAge;
	 int		mLifespan;
	 bool		mIsDead;
};