//  ***************************************************************
//  ISoundManager - Creation date: 05/12/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef ISoundManager_h__
#define ISoundManager_h__

#ifndef _OLDLIBRARY
	#include "MiscUtils.h"
#else
	#include "all.h" //my old basecode for dungeon scroll
#endif
#include <cassert>
#include <map>
using namespace std;

#define C_SOUND_NONE -1

class ISoundManager
{
public:
	virtual ~ISoundManager() {};

	virtual int PlayMixed(const char *p_fname)=0;
	virtual int Play(const char *p_fname)=0;
	virtual int PlayMusic(const char *p_fname, int i_loop_count)=0;

	virtual bool Init()=0;
	virtual void UpdateSounds()=0; //called every frame, only need for fading/3d
	virtual void MuteAll(bool b_new)=0;

	virtual void Kill()=0; //kills everything, unloads all sounds and main driver
	virtual void KillMusic()=0; //unloads just the music

	virtual int PlayLooping(const char *p_fname)=0;
	virtual void KillChannel(int i_channel)=0;
	virtual bool IsInitted()=0;
	virtual bool IsMusicPlaying()=0;

	virtual void SetSpeedFactor(int soundID, float mod) { assert(!"Not implemented with this driver yet");}; //a mod of 2 would play twice as fast
	virtual void SetVolume(int soundID, float volume) { assert(!"Not implemented with this driver yet");};
	virtual void AddEffect(int soundID, int effectID, float parmA, float parmB, float parmC) { assert(!"Not implemented with this driver yet");};
	virtual	void RemoveAllEffects(int soundID) { assert(!"Not implemented with this driver yet");};
	virtual void SetPan(int soundID, float pan) { assert(!"Not implemented with this driver yet");};//-1 to 1
	virtual bool IsSoundPlaying(int soundID) {assert(!"Not implemented with this driver yet"); return false;}
	virtual void SetPaused(int soundID, bool bPaused) {assert(!"Not implemented with this driver yet"); return;}
	virtual void SetPriority(int soundID, int priority){}; //between 0 and 255

	enum
	{
		C_EFFECT_NONE = -1,
		C_EFFECT_FADE = 0,

		//add more above here
		C_EFFECT_COUNT
	};

protected:
	

private:
};


#endif // ISoundManager_h__


