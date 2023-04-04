
m_soundID = C_SOUND_NONE;

function OnInit() //run upon initialization
	this:Data():SetIfNull("File", "audio/crap.ogg");

	if (GameIsActive()) then
		//this is a real game, not the editor
		//this trigger will call OnTriggerInside every 2000 MS, plus the enter and exit callbacks
		this:SetTrigger(C_TRIGGER_TYPE_REGION_IMAGE, 0, C_TRIGGER_BEHAVIOR_PULSE, 200);
	end


end

function CreateSoundIfNeeded()
	if (m_soundID == C_SOUND_NONE) then
		m_soundID = GetSoundManager:PlayLooping(this:Data():Get("File"));
	end
end

function UpdateSound()

  	size = this:GetSizeX();
	  if (this:GetSizeY() > size) then size = this:GetSizeY(); end;
	//LogMsg("Size is",size);
	this:UpdateSoundByPosition(m_soundID, 100, size/2, 1);
end

function OnTriggerEnter(Ent)

  //LogMsg("Entering sound region.  Entity touched us at " .. tostring(Ent:GetPos()));

	CreateSoundIfNeeded();
	UpdateSound();
	
end

function OnTriggerExit(Ent)

	//LogMsg("Leaving region");
	KillSound();
end

function OnTriggerInside(Ent)
	CreateSoundIfNeeded();
	UpdateSound();
end

function OnPostInit() //run during the entity's first logic update

end

function KillSound()

	if (m_soundID != C_SOUND_NONE) then
			GetSoundManager:Kill(m_soundID);
			m_soundID = C_SOUND_NONE;	
		end
end

function OnKill() //run when removed
	KillSound();
end



