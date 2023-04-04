function OnInit()

	this:Data():SetIfNull("File", "audio/crap.ogg");
	this:Data():SetIfNull("bFadeInNew", "1");
	this:Data():SetIfNull("bFadeOutOld", "1");


	if (GameIsActive()) then
		//this is a real game, not the editor
		//this trigger will call OnTriggerInside every 2000 MS, plus the enter and exit callbacks
		this:SetTrigger(C_TRIGGER_TYPE_REGION_IMAGE, 0, C_TRIGGER_BEHAVIOR_PULSE, 2000);
	end

end


function OnTriggerEnter(Ent)

  //LogMsg("Entering sound region.  Entity touched us at " .. tostring(Ent:GetPos()));

end

function OnTriggerExit(Ent)

	//LogMsg("Leaving region");
end

function OnTriggerInside(Ent)

	//LogMsg("Inside region. music is currently " .. g_activeMusicFilename);
 	g_SmartAmbience:SetMusic(this:Data():Get("File"), this:Data():Get("bFadeInNew"), this:Data():Get("bFadeOutOld"));

end