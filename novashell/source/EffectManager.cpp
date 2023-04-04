#include "AppPrecomp.h"
#include "EffectManager.h"
#include "MovingEntity.h"

#include "linearparticle/sources/L_ParticleSystem.h"

EffectManager g_EffectManager;

EffectManager::EffectManager()
{
	L_ParticleSystem::init(20000);
}

void EffectManager::Reset()
{
	m_particleMap.clear();
}

EffectManager::~EffectManager()
{
	L_ParticleSystem::deinit();
}

L_Particle * EffectManager::GetParticleByName(const string &name)
{
	particleMap::iterator itor = m_particleMap.find(name);

	if (itor != m_particleMap.end())
	{
		//bingo!
		return &(itor->second);
	}
	return NULL; //doesn't exist
}

L_Particle * EffectManager::CreateParticle(const string &name, const string &filename, int lifeMS)
{
	particleMap::iterator itor = m_particleMap.find(name);

	if (itor != m_particleMap.end())
	{
		//already exists?!
		LogError("Can't create particle type %s, it already exists.", name.c_str());
		return NULL;
	}
	
	string fname = C_DEFAULT_SCRIPT_PATH + filename;
	g_VFManager.LocateFile(fname);

	CL_Surface *pSurf = new CL_Surface(fname);
	pSurf->set_alignment(origin_center);
	m_particleMap[name] = L_Particle(pSurf, lifeMS);
	L_Particle *pParticle = &m_particleMap[name];;

	pParticle->set_delete_surface(true);
	return pParticle;
}

L_ExplosionEffect * EffectManager::CreateEffectExplosion(int period_t, int min_particles_t, int max_particles_t, float explosion_level_t)
{
	return new L_ExplosionEffect(0,0,period_t, min_particles_t, max_particles_t, explosion_level_t);
}


/*
Object: EffectManager
Allows interesting visual effects to be created and attached to entities.

About:

This is a global object that can always be accessed.

When it's time to put some fire in your brazier, or smoke in your chimney, it's time for particle systems.

The EffectManager is based around *Wong Chin Foo*'s LinearParticle system.  Although the interface has been somewhat streamlined to match the rest of Novashell, you might want to check out his webpage at (http://galaxist.net/file/linear_p/latest_doc) to glean some info on its inner-workings.

How it works:

The effect system is modular to allow more flexibility.  You plug and play different pieces together to get the effect you want.

Particle - A global template <Particle> that is shared between effects.
MotionController - Every <Particle> has a <MotionController> that can be accessed to setup special movement.
EffectBase - All effect emitters are derived from <EffectBase> and contain its functions too.
EffectExplosion - A type of particle emitter good for explosions.  Create an <EffectExplosion>, attach one or more particles, then assign it to an entity with <Entity:AddEffect>.

Group: Member Functions

func: CreateParticle
(code)
Particle CreateParticle(string name, string imageFileName, number lifeMS)
(end)
Allows you to create a particle template that can be referenced by name.  If you change any parameters later on this particle, all particle systems using it will be modified.

Usage:
(code)
local particle = GetEffectManager:GetParticleByName("smoke_brazier");

if (not particle) then
	//doesn't exist?  We'll need to create it then.
	particle = GetEffectManager:CreateParticle("smoke_brazier", "effects/media/explosion.png", 500);
end

//now we can use "particle" anywhere needed.

end
(end)

Parameters:
name - A name that can be used to access this particle in the future.  It will give an error if the name already exists.
imageFileName - An image to use for the particles visual.  Generally a 32 bit .png is used.
lifeMS - How long this type of particle 'lives' before disappearing.

Returns:

A handle to a global <Particle> object created that can be used to make additional changes.

func: GetParticleByName
(code)
Particle GetParticleByName(string name)
(end)

Returns:
The shared <Particle> previously created with the same name, or nil if it doesn't exist yet.

func: CreateEffectExplosion
(code)
EffectExplosion CreateEffectExplosion(number periodMS, number minParticles, number maxParticles, number initialSpeed)
(end)
Creates a new <EffectExplosion> object.  
You must use <Entity::AddEffect> to add it to an entity.
When its containing entity is destroyed it is destroyed also.  Trying to attach the same explosion to two or more entities will explode your computer, don't do it.

Parameters:
periodMS - interval between two emissions in milliseconds
minParticles - The minimum number of particles to be emitted during a period
maxParticles - The maximum number of particles to be emitted during a period
intialSpeed - The initial velocity that particles have when created

Returns:
The new <EffectExplosion> object, ready to be attached to an entity with <Entity::AddEffect>.
*/