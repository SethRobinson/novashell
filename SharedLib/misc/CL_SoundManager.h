//  ***************************************************************
//  CL_SoundManager - Creation date: 05/12/2006
//  -------------------------------------------------------------
//  Copyright 2007: Robinson Technologies - Check license.txt for license info.
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef CL_SoundManager_h__
#define CL_SoundManager_h__

#include "ISoundManager.h"
#include <API/sound.h>

//these are saved and cached out
class SoundResource
{
public:
	
	CL_SoundBuffer m_buffer;
};

class SoundSession
{
public:
	SoundSession()
	{
		m_bIsPaused = false;
	}
	~SoundSession()
	{
		m_session.stop();
	}
	CL_SoundBuffer_Session m_session;
	bool m_bIsPaused;
};

typedef std::map<string, SoundResource> soundResourceMap;

typedef std::map<int, SoundSession> soundSessionMap;

class CL_SoundManager:public ISoundManager
{
public:
	CL_SoundManager();
	virtual ~CL_SoundManager();

	virtual int PlayMixed(const char *p_fname);
	virtual int Play(const char *p_fname);
	virtual int PlayMusic(const char *p_fname, int i_loop_count);

	virtual bool Init();
	virtual void UpdateSounds(); //called every frame, only need for fading/3d
	virtual void MuteAll(bool b_new);

	virtual void Kill(); //kills everything, unloads all sounds and main driver
	virtual void KillMusic(); //unloads just the music
	void SetPaused(int soundID, bool bPaused);

	virtual int PlayLooping(const char *p_fname);
	virtual void KillChannel(int i_channel);
	virtual bool IsInitted() {return m_bInitted;}
	virtual bool IsMusicPlaying();

	virtual void SetSpeedFactor(int soundID, float mod); //a mod of 2 would play twice as fast
	void AddEffect(int soundID, int effectID, float parmA, float parmB, float parmC);
	void SetVolume(int soundID, float volume);
	void RemoveAllEffects(int soundID);
	virtual void SetPan(int soundID, float pan); //-1 to 1
	virtual bool IsSoundPlaying(int soundID);
	virtual void SetPriority(int soundID, int priority);

protected:

	SoundResource * LocateSound(const char *pFname);
	int GetUniqueID() {return ++m_baseID;}
	SoundSession * GetSessionFromID(int id);

	soundResourceMap m_soundResources;
	soundSessionMap m_soundSessions;

	CL_SoundBuffer_Session m_music;
	int m_baseID;
	bool m_bInitted;

private:
};

#endif // CL_SoundManager_h__
