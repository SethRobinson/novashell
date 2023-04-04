/* -------------------------------------------------
* Programmer(s):  Seth A. Robinson (seth@rtsoft.com): 
* Created Wednesday, August 24, 2004 7:24:40 PM
*/

#ifndef FMSoundManager_HEADER_INCLUDED // include guard
#define FMSoundManager_HEADER_INCLUDED  // include guard

#ifndef _OLDLIBRARY
	#include "MiscUtils.h"
#else
	#include "all.h" //my old basecode for dungeon scroll

#endif

#include "ISoundManager.h"
#include "fmod.h"

class SoundEffect
{
public:

	SoundEffect(int channelID): m_channelID(channelID)
	{
		m_eventType = ISoundManager::C_EFFECT_NONE;
	}

	void FadeToVolume(float targetVol, int durationMS);
	bool Update(unsigned int timeMS); //return false to delete it

protected:

	int m_eventType;
	float m_target;
	float m_original;
	int m_channelID;
	unsigned int m_durationMS;
	unsigned int m_startTimeMS;
};

class ChannelSession
{
public:
	vector<SoundEffect> m_effects;
};

typedef std::map<int, ChannelSession> channelMap;

class CFMSound
{
public:
    CFMSound()
    {
        p_sample = NULL;
    }
    FSOUND_SAMPLE *p_sample;
    string st_fname;
};


class CFMSoundManager: public ISoundManager
{
public:

    CFMSoundManager();
    virtual ~CFMSoundManager();
    
    virtual int PlayMixed(const char *p_fname);
    virtual int Play(const char *p_fname);
    virtual int PlayMusic(const char *p_fname, int i_loop_count);
    
    virtual bool Init();
    virtual void UpdateSounds(); //called every frame, only need for fading/3d
    virtual void MuteAll(bool b_new);

    virtual void Kill(); //kills everything, unloads all sounds and main driver
    virtual void KillMusic(); //unloads just the music

    virtual int PlayLooping(const char *p_fname);
    virtual void KillChannel(int i_channel);
    virtual bool IsInitted() {return m_b_ready;}
    virtual bool IsMusicPlaying();
	void Preload(const char *p_fname);

	virtual void SetSpeedFactor(int soundID, float mod);
	virtual void SetVolume(int soundID, float volume);
	virtual void AddEffect(int soundID, int effectID, float parmA, float parmB, float parmC);
	virtual	void RemoveAllEffects(int soundID);
	virtual void SetPan(int soundID, float pan);
	virtual bool IsSoundPlaying(int soundID);
	virtual void SetPaused(int soundID, bool bPause);
	virtual void SetPriority(int soundID, int priority);


private:

	FSOUND_SAMPLE * GetSoundSampleByName(const char *p_fname);
	CFMSound * PreloadSound(const char *p_fname);
	ChannelSession * CFMSoundManager::GetEffectsDataForChannel(int channelID);

  bool m_b_ready;

  FMUSIC_MODULE *m_p_music_mod; //used only for music
  FSOUND_SAMPLE *m_p_music_sample; //used only for music
  vector<CFMSound> m_a_sounds; //array of our loaded sounds

  channelMap m_channelMap;
};

#endif                  // include guard
