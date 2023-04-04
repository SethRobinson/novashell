#include "FMSoundManager.h"
#include <fmod_errors.h> 

#ifndef _OLDLIBRARY
	#include "CL_VirtualFileManager.h"
	//well, there are many betters way to get access to the virtual file manager but this will do
	extern CL_VirtualFileManager g_VFManager;
#endif

void SoundEffect::FadeToVolume(float targetVol, int durationMS)
{
	assert(m_eventType == ISoundManager::C_EFFECT_NONE && "You can't use more than one effect on the same class, make a new effect");
	
	m_target = targetVol;
	m_durationMS = durationMS;
	m_original = float(FSOUND_GetVolume(m_channelID))/255;
	m_eventType = ISoundManager::C_EFFECT_FADE;
	
	#ifndef _OLDLIBRARY
		m_startTimeMS = CL_System::get_time();
	#else
		m_startTimeMS = 0;
		assert(!"This isn't supported without clanlib");
	#endif
}

bool SoundEffect::Update(unsigned int timeMS)
{
#ifndef _OLDLIBRARY
	switch (m_eventType)
	{
	
	
	case ISoundManager::C_EFFECT_FADE:
		//process the fading here
		if (timeMS > m_startTimeMS+m_durationMS)
		{
			//we're done
			return false; //you can erase us now
		} else
		{
		
			
			//still fading.  
			float lerpAmount = (float(timeMS-m_startTimeMS))/float(m_durationMS);
			FSOUND_SetVolume(m_channelID, int(Lerp(m_original, m_target, lerpAmount)*255));
		}
		break;

	default:

		LogError("Unknown effect type: %d", m_eventType);
	}
#endif
	return true; //keep processing this, don't delete it
}

CFMSoundManager::CFMSoundManager()
{
    m_b_ready = false;
    m_p_music_mod = NULL;
    m_p_music_sample = NULL;
}

void CFMSoundManager::MuteAll(bool b_new)
{
  FSOUND_SetMute(FSOUND_ALL, b_new);
}

bool CFMSoundManager::Init()
{
	bool bFirstTime = !m_b_ready;
    Kill();

  if (FSOUND_GetVersion() < FMOD_VERSION)
    {
        LogError("You are using the wrong DLL version!  You should be using FMOD %.02f\n", FMOD_VERSION);
        return false;
    }
    
    /*
        INITIALIZE
    */
   //FSOUND_SetBufferSize(200);  //causes major lag
       
       
       if (!FSOUND_Init(44100, 8, 0))
    {
        LogError("%s\n", FMOD_ErrorString(FSOUND_GetError()));
        return false;
    }
	if (bFirstTime)
	{
	   LogMsg("FMOD V%.2f Sound System Initialized", FSOUND_GetVersion());
	}
    
    return m_b_ready = true;
}


CFMSoundManager::~CFMSoundManager()
{
   Kill(); //deallocate everything we had going
}

int CFMSoundManager::Play(const char *p_fname)
{
  return PlayMixed(p_fname);
}

bool CFMSoundManager::IsSoundPlaying(int soundID)
{
	return FSOUND_IsPlaying(soundID) != 0;
}

//case sensitive!  Always use lower case.
FSOUND_SAMPLE * CFMSoundManager::GetSoundSampleByName(const char *p_fname)
{

    for (unsigned int i=0; i < m_a_sounds.size(); i++)
    {
         if (strcmp(m_a_sounds[i].st_fname.c_str(), p_fname) == 0)
        {
            //found it
            return m_a_sounds[i].p_sample;
        }
    }
   //couldn't find it.
    return NULL;
}


CFMSound * CFMSoundManager::PreloadSound(const char *p_fname)
{

#ifndef _OLDLIBRARY

	string fname = p_fname;

if (!g_VFManager.LocateFile(fname))
	{
		LogError("Unable to locate audio file %s.", p_fname);
		return NULL;
	}
#endif	

	m_a_sounds.push_back(CFMSound());

   int i_index = m_a_sounds.size()-1; //only works with push back, later we'll add insert where
   //LogMsg("Sound array is now %d.", i_index);
   //a vector has a null sound or something like that
#ifndef _OLDLIBRARY

   m_a_sounds[i_index].p_sample = FSOUND_Sample_Load(FSOUND_FREE, fname.c_str(), 0,0, 0);
#else
   m_a_sounds[i_index].p_sample = FSOUND_Sample_Load(FSOUND_FREE, p_fname, 0,0, 0);
#endif

   m_a_sounds[i_index].st_fname = p_fname; //remember this for later
   return &m_a_sounds[i_index]; //do NOT save this and use it later, the vector could be resized which may
   //invalidate this pointer!
}

void CFMSoundManager::Preload(const char *p_fname)
{
	PreloadSound(p_fname);
}

int CFMSoundManager::PlayMixed(const char *p_fname)
{
    if (!IsInitted()) return -1;

 FSOUND_SAMPLE  *p_sample = GetSoundSampleByName(p_fname);

 if (!p_sample)
 {
   //load it, it has not been preloaded yet.
   p_sample = PreloadSound(p_fname)->p_sample;

   if (!p_sample)
   {
       LogError("Unable to load sound %s.", p_fname);
       return -1;
   }
 }
     
 int i_channel = FSOUND_PlaySound(FSOUND_FREE, p_sample);
 FSOUND_SetLoopMode(i_channel, FSOUND_LOOP_OFF);
 FSOUND_SetPriority(i_channel, 0);
 return i_channel;
}


int CFMSoundManager::PlayLooping(const char *p_fname)
{
 if (!IsInitted()) return -1;
 int i_channel = PlayMixed(p_fname);
 FSOUND_SetLoopMode(i_channel, FSOUND_LOOP_NORMAL);
 FSOUND_SetPriority(i_channel, 100);
 return i_channel;
}

void CFMSoundManager::KillChannel(int i_channel)
{
	FSOUND_SetReserved(i_channel, false); //just in case it was reserved

	FSOUND_StopSound(i_channel);
}

void CFMSoundManager::SetSpeedFactor(int soundID, float mod)
{
	if (!IsInitted()) return;
	FSOUND_SetFrequency(soundID, int((22050*2)*mod));
}

void CFMSoundManager::SetPan(int soundID, float pan)
{
	if (!IsInitted()) return;
	FSOUND_SetPan(soundID, int((pan+1)*128));
}

void CFMSoundManager::SetPriority(int soundID, int priority)
{
	if (!IsInitted()) return;
    FSOUND_SetPriority(soundID, priority);
}

ChannelSession * CFMSoundManager::GetEffectsDataForChannel(int channelID)
{
	channelMap::iterator itor = m_channelMap.find(channelID);
	if (itor != m_channelMap.end())
	{
		return &(itor->second);
	}
	return NULL;
}

void CFMSoundManager::AddEffect(int soundID, int effectID, float parmA, float parmB, float parmC)
{
	//get a handle to its effect stuff
	if (!FSOUND_IsPlaying(soundID))
	{
		LogMsg("SoundID %d isn't playing, won't add effect.", soundID);
		return;
	}

	ChannelSession *pChanSession = GetEffectsDataForChannel(soundID);

	//else, we need to create it

	if (!pChanSession)
	{
		pChanSession = &m_channelMap[soundID]; //this will create it
	}
	
	switch (effectID)
	{
	case C_EFFECT_FADE:

		{
			SoundEffect e(soundID);
			SetVolume(soundID, parmA*255);
			e.FadeToVolume(parmB, int(parmC));
			pChanSession->m_effects.push_back(e);
			break;
		}

	default:
		LogError("Unknown sound effect: %d", effectID);
		return;
	}

}

void CFMSoundManager::RemoveAllEffects(int soundID)
{
	
	ChannelSession *pSession = GetEffectsDataForChannel(soundID);

	if (pSession)
	{
		pSession->m_effects.clear();
	}
	
}

void CFMSoundManager::SetVolume(int soundID, float volume)
{
	if (!IsInitted()) return;
	FSOUND_SetVolume(soundID, int(volume*255));
}

void CFMSoundManager::SetPaused(int soundID, bool bPause)
{
	if (!IsInitted()) return;

	FSOUND_SetPaused(soundID, bPause);

	if (bPause)
	{
		FSOUND_SetReserved(soundID, true);
	}
}

int CFMSoundManager::PlayMusic( const char *p_fname, int i_loop_count)
{
    if (!IsInitted()) return -1;

    KillMusic();
    
    m_p_music_sample = FSOUND_Sample_Load(FSOUND_FREE, p_fname, 0,0, 0);
    
	if (!m_p_music_sample)
	{
		LogMsg("PlayMusic: Error loading %s: %s\n", p_fname, FMOD_ErrorString(FSOUND_GetError()));

	}
	int i_channel = FSOUND_PlaySound(FSOUND_FREE, m_p_music_sample);
    
    if (i_loop_count <= 1)
    {
        
        FSOUND_SetLoopMode(i_channel, FSOUND_LOOP_OFF);
    } else
    {
        FSOUND_SetLoopMode(i_channel, FSOUND_LOOP_NORMAL);
     }
    
    return 1;
}

bool CFMSoundManager::IsMusicPlaying()
{
    if (m_p_music_mod || m_p_music_sample)
    {
        //TODO: see if it's still actually playing
        return true;
    }

    return false;
}

void CFMSoundManager::KillMusic()
{
    if (m_p_music_mod)
    {
        FMUSIC_FreeSong(m_p_music_mod);
        m_p_music_mod = NULL;
    }
    
    if (m_p_music_sample)
    {
        FSOUND_Sample_Free(m_p_music_sample);
        m_p_music_sample = NULL;
    }
}

void CFMSoundManager::Kill()
{
   m_channelMap.clear();
	
	if (IsInitted())
    {
        KillMusic();
        FSOUND_Close();
    }
    m_b_ready = false;
    m_a_sounds.clear();
}

void CFMSoundManager::UpdateSounds()
{
	if (!m_b_ready) return;

#ifndef _OLDLIBRARY
	channelMap::iterator itor;

	unsigned int curTime = CL_System::get_time();
	bool bDelete;

	for (itor = m_channelMap.begin(); itor != m_channelMap.end();)
	{

		if (FSOUND_IsPlaying(itor->first))
		{
			//LogMsg("Updating soundID %d", itor->first);

			//process its filters
			for (unsigned int i=0; i < itor->second.m_effects.size(); i++)
			{
				bDelete = !itor->second.m_effects[i].Update(curTime);
			}
			//the sound is still playing
		} else
		{
			//sound is dead, might as well kill it, right?
			//delete it I guess
			channelMap::iterator itorSave = itor;
			itorSave++;
			m_channelMap.erase(itor);
			itor = itorSave;
			continue;
		}
		
		itor++;
	}
#endif
}