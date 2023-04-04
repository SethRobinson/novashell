#include "AppPrecomp.h"
#include "EffectManagerBindings.h"
#include "EffectManager.h"
#include "linearparticle/sources/L_ExplosionEffect.h"


#ifndef WIN32
//windows already has this in the precompiled header for speed, I couldn't get that to work on mac..
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#endif

using namespace luabind;

string EffectManagerToString(EffectManager * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "EffectManager");
	return string(stTemp);
}
string ParticleToString(L_Particle * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "EffectManager");
	return string(stTemp);
}

string EffectExplodeToString(L_ExplosionEffect * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "EffectExplode");
	return string(stTemp);
}

string ParticleEffectToString(L_ParticleEffect * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "ParticleEffect");
	return string(stTemp);
}

string MotionControllerToString(L_ParticleEffect * pObj)
{
	char stTemp[256];
	sprintf(stTemp, "MotionController");
	return string(stTemp);
}

void ParticleSetColor(L_Particle * pParticle, const CL_Color &col)
{
	pParticle->set_color(L_Color(col));
}
void ParticleSetColoring1(L_Particle * pParticle, const CL_Color &col,  float num)
{
	pParticle->coloring1(L_Color(col), num);
}
void ParticleSetColoring2(L_Particle * pParticle, const CL_Color &col, const CL_Color &col2, float num)
{
	pParticle->coloring2(L_Color(col), L_Color(col2), num);
}

void ParticleEffectSetOffset(L_ParticleEffect * pParticleEffect, const CL_Vector2 &v)
{
	pParticleEffect->set_offset(v.x, v.y);
}

void ParticleEffectSetAdditionalVector(L_ParticleEffect * pParticleEffect, const CL_Vector2 &v)
{
	pParticleEffect->set_addit_vector(L_Vector(v.x, v.y));
}

L_MotionController * ParticleGetMotionController(L_Particle * pParticle)
{
	if (!pParticle->get_motion_controller())
	{
		//create it
		pParticle->set_motion_controller(new L_MotionController);
		pParticle->set_delete_motion_controller(true);
	}
	
	return pParticle->get_motion_controller();
}

void MotionController2DAcceleration(L_MotionController * pMotion, const CL_Vector2 &vec)
{
	pMotion->set_2d_acceleration(L_Vector(vec));
}

void luabindEffectManager(lua_State *pState)
{
	module(pState)
		[
			class_<L_MotionController>("MotionController")
			.def("__tostring", &MotionControllerToString)
			.def("Set1DAcceleration", &L_MotionController::set_1d_acceleration)
			.def("Set2DAcceleration", &MotionController2DAcceleration)
			.def("SetMaxSpeed", &L_MotionController::set_speed_limit)

			,class_<L_Particle>("Particle")
			.def("__tostring", &ParticleToString)
			.def("SetColor", &ParticleSetColor)
			.def("SetColoring1", &ParticleSetColoring1)
			.def("SetColoring2", &ParticleSetColoring2)
			.def("GetMotionController", &ParticleGetMotionController)
			.def("SetSizing2", &L_Particle::sizing2)
			.def("SetRotationEveryCycle", &L_Particle::rotating1)
			.def("SetRotationOverLife", &L_Particle::rotating2)
			.def("SetRotationRandom", &L_Particle::rotating3)
			.def("SetRotationFollowVelocity", &L_Particle::rotating4)
			.def("SetBlendMode", &L_Particle::set_blending_mode)

			,class_<L_ParticleEffect>("EffectBase")
			.def("__tostring", &ParticleEffectToString)
			.def("AddParticle", &L_ParticleEffect::add)
			.def("SetOffset", &ParticleEffectSetOffset)
			.def("SetAdditionalVector", &ParticleEffectSetAdditionalVector)
			.def("SetLife", &L_ParticleEffect::set_life)
			.def("SetSizeDistortion", &L_ParticleEffect::set_size_distortion)
			.def("SetLifeDistortion", &L_ParticleEffect::set_life_distortion)
			.def("SetRandomRotation", &L_ParticleEffect::set_par_random_rotation)
			.def("SetActive", &L_ParticleEffect::trigger)

			,class_<L_ExplosionEffect, L_ParticleEffect>("EffectExplode")
			.def("__tostring", &EffectExplodeToString)
			.def("SetSpeedDistortion", &L_ExplosionEffect::set_speed_distortion)

			,class_<EffectManager>("EffectManager")
			.def("__tostring", &EffectManagerToString)
			.def("GetParticleByName", &EffectManager::GetParticleByName)
			.def("CreateParticle", &EffectManager::CreateParticle)
			.def("CreateEffectExplosion", &EffectManager::CreateEffectExplosion)

		];
}
