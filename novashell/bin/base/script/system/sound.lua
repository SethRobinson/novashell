//Code for controlling music and ambient sounds

SmartSound = class();

function SmartSound:__init()

    //init default values
    
    self.musicID = C_SOUND_NONE;
		self.lastMusicFileName = "";
	
end

function SmartSound:GetActiveAudioFileName()
	return self.lastMusicFileName;
end

function SmartSound:FadeOutCurrentMusic()

	if (self.musicID != C_SOUND_NONE) then
		Schedule(4500, C_ENTITY_NONE, "GetSoundManager:Kill(" .. self.musicID .. ");");
		GetSoundManager:RemoveAllEffects(self.musicID);
		GetSoundManager:AddEffect(self.musicID, C_EFFECT_FADE, 1.0, 0.0, 2000);
	    self.musicID = C_SOUND_NONE;
	    return true; //actually fading something out
	else
		return false; //not currently playing anything
	end

end

function SmartSound:StopCurrentMusic()

	if (self.musicID != C_SOUND_NONE) then
		GetSoundManager:Kill(self.musicID);
	    self.musicID = C_SOUND_NONE;
		  return true; //actually stopped something
	else
		return false; //nothing was playing
	end
end

function SmartSound:FadeInLastPlayedMusic()
		self:SetMusic(self.lastMusicFileName, true, false);
end

function SmartSound:RestartLastPlayedMusic()
		self:SetMusic(self.lastMusicFileName, false, false);
end


//send a blank filename to set music to none
function SmartSound:SetMusic(fileName, bFadeInNew, bFadeOutOld)

	if (self.musicID != C_SOUND_NONE and self.lastMusicFileName == fileName) then
		return; //don't need to set the music, it's already right
	end
	
	
	//kill any existing music?

	if (toboolean(bFadeOutOld)) then
		self:FadeOutCurrentMusic();
	else
		self:StopCurrentMusic();
	end

	
	//setup the new music
	
	if (g_musicEnabled == false) then
		return;
	end

	self.lastMusicFileName = fileName;
	
	if (fileName != "") then
	
		self.musicID = GetSoundManager:PlayLooping(fileName);
		if (toboolean(bFadeInNew)) then
			GetSoundManager:AddEffect(self.musicID, C_EFFECT_FADE, 0.0, 1.0, 3000);
		end
	end
	
end

//init some globals to use, one for music, one for sfx loops
g_SmartSound = SmartSound();
g_SmartAmbience = SmartSound();
