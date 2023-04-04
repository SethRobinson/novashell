// ***************************************************************
//  EffectManager   date: 03/13/2007
//  -------------------------------------------------------------
//  Programmer(s): Seth A. Robinson (seth@rtsoft.com)
//  -------------------------------------------------------------
//  Copyright (C) 2007 - Robinson Technologies
// ***************************************************************

#ifndef EffectManager_h__
#define EffectManager_h__

#include "linearparticle/sources/L_Particle.h"

class L_ExplosionEffect;

typedef std::map<string, L_Particle> particleMap;

class EffectManager
{
public:
	EffectManager();
	~EffectManager();
	
	void Reset();

	L_Particle *GetParticleByName(const string &name);
	L_Particle *CreateParticle(const string &name, const string &filename, int lifeMS);

	L_ExplosionEffect * CreateEffectExplosion(int period_t, int min_particles_t, int max_particles_t, float explosion_level_t);

private:

	particleMap m_particleMap;
};

extern EffectManager g_EffectManager;
#endif // EffectManager_h__

