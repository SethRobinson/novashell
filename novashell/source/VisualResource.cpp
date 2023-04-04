#include "AppPrecomp.h"
#include "VisualResource.h"
#include "VisualProfile.h"

VisualResource::VisualResource()
{
	m_pResourceManager = NULL;
}

VisualResource::~VisualResource()
{
	for (unsigned int i=0; i < m_profileVec.size(); i++)
	{
		delete m_profileVec[i];
	}

	m_profileVec.clear();

	SAFE_DELETE(m_pResourceManager);
}

VisualProfile * VisualResource::GetProfile(const string &profileName)
{
	VisualProfile *pProf = NULL;

	//does it already exist?
	for (unsigned int i=0; i < m_profileVec.size(); i++)
	{
		if (m_profileVec[i]->GetName() == profileName)
		{
			//found it
			return m_profileVec[i];
		}
	}

	//it's not here, add it
	pProf = new VisualProfile();
	if (!pProf || !pProf->Init(this, profileName))
	{
		LogMsg("Error initializing profile %s", profileName.c_str());
		SAFE_DELETE(pProf);
		return NULL;
	}
	m_profileVec.push_back(pProf);

	return pProf;
}

bool VisualResource::Init(const string &fileName)
{
	try
	{
		m_pResourceManager = new CL_ResourceManager(fileName);
	} catch(CL_Error error)
	{
		LogMsg(error.message.c_str());
		return false;
	}

	m_fileName = fileName;
	return true;
}

void VisualResource::CopyFromProfilesToDocument(CL_DomDocument &document)
{
	for (unsigned int i=0; i < m_profileVec.size(); i++)
	{
		m_profileVec[i]->UpdateToDocument(document);
	}
}

void VisualResource::Save()
{
	LogMsg("Saving %s", m_fileName.c_str());

	CL_DomDocument document = m_pResourceManager->get_resource(m_profileVec[0]->GetName()).get_element().get_owner_document();
	
	CL_InputSourceProvider *provider = new CL_InputSourceProvider_File(CL_String::get_path(m_fileName));
	document.load(provider->open_source(CL_String::get_filename(m_fileName)), true, false);
	//run through all settings that may have changed directly from our profile system

	CopyFromProfilesToDocument(document);

	CL_OutputSource *output = new CL_OutputSource_File(m_fileName);
	document.save(output, true, false);

}




//natural docs stuff
/*
Object: SoundManager
Controls all game audio.

About:

This is a global object that can always be accessed.

Usage:

(code)
GetSoundManager:Play("audio/warp.ogg");
(end)

Group: Member Functions

func: Play
(code)
number Play(string fileName)
(end)
Plays a sound.  Audio files are automatically cached after use for speed.

Parameters:

fileName - A .wav or .ogg sound file to play.

Returns:

A numerical handle to the sound created.  Save it if you want to <Kill> it later or adjust its properties.

func: PlayLooping
(code)
number PlayLooping(string fileName)
(end)
Like <Play> but the sound loops for eternity.  You must use <Kill> to make it stop.

Parameters:

fileName - A .wav or .ogg sound file to play.

Returns:

A numerical handle to the sound created.  Save it if you want to <Kill> it later or adjust its properties.

func: SetSpeedFactor
(code)
nil SetSpeedFactor(number soundID, number speedMod)
(end)
Allows you to adjust the speed/pitch of a sound.

Usage:
(code)
local soundID = this:PlaySoundPositioned("~/audio/quack.ogg");
//randomize the pitch
GetSoundManager:SetSpeedFactor(soundID, 0.90 + (random() * 0.2) );
(end)

Parameters:

soundID - A valid sound handle.
speedMod - A number indicating the speed modification, 1 is normal speed, 1.1 is faster, 0.9 is slower.

func: SetVolume
(code)
nil SetVolume(number soundID, number volume)
(end)

Parameters:

soundID - A valid sound handle.
volume - The new volume of the sound.  0 to mute, 1 for max volume.

func: SetPan
(code)
nil SetPan(number soundID, number pan)
(end)

Parameters:

soundID - A valid sound handle.
pan - A range between -1 and 1.  (0 would be normal stereo with neither speaker getting favored)

func: SetPaused
(code)
nil SetPaused(number soundID, boolean bPaused)
(end)

Parameters:

soundID - A valid sound handle.
bPaused - If true, sound will stop playing, but will remain ready to start playing again from its current position with another <SetPaused> command.

Note:

You must <Kill>() this sound or set its priority to 0 once you're done with it so it can be properly garbage collected.

func: SetPriority
(code)
nil SetPriority(number soundID, number priority)
(end)

A sound with a higher priority won't be interrupted by a sound with a lower priority in cases of limited sound channels, as when using FMOD.

Parameters:

soundID - A valid sound handle.
priority - A number between 0 (low) to 255 (high) priority.

func: IsPlaying
(code)
boolean IsPlaying(number soundID)
(end)

Parameters:

soundID - A valid sound handle.

Returns:

True if the specified sound is currently playing.

func: Kill
(code)
nil Kill(number soundID)
(end)

Stops a sound from playing.

Parameters:

soundID - A valid sound handle.

func: MuteAll
(code)
nil MuteAll(boolean bMuteActive)
(end)

Easy way to turn the volume of ALL sounds off and on at the same time, without actually stopping the sounds.

Parameters:

bMuteActive - If true, the sound is muted.  If false, sounds can be heard.

Section: Related Constants

Group: C_SOUND_CONSTANTS
Used with sound functions.

constant: C_SOUND_NONE
Sound functions return this if no sound was created.

*/