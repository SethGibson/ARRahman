#pragma once
#include "Particle.h"
#include "cinder/Channel.h"
#include "cinder/Perlin.h"
#include <list>

class ParticleController {
 public:
	//ParticleController();
	//void applyForce( float zoneRadius, float lowerThresh, float higherThresh, float attractStrength, float repelStrength, float orientStrength );
	//void pullToCenter( const ci::Vec3f &center );
	//void update( bool flatten );
	//void draw();
	//void addParticles( int amt );
	//
	//std::list<Particle>	mParticles;
	//ci::Vec3f mParticleCentroid;
	//int mNumParticles;

	 ParticleController();
	 //void repulseParticles();
	 //void pullToCenter();
	// void applyPerlin(const ci::Perlin &perlin);
	// void update(const ci::Channel32f &channel, const ci::ivec3 &mouseLoc);
	 void update();
	 void draw();
	 //void addParticles(int amt, const ci::ivec2 &mouseLoc, const ci::vec2 &mouseVel);
	 void addParticles(int amt);


	 std::list<Particle>	mParticles;

};