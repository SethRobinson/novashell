function OnInit()

	this:Data():SetIfNull("File", "audio/crap.ogg");
	
		//this is a real game, not the editor
		//this trigger will call OnTriggerInside every 2000 MS, plus the enter and exit callbacks
		this:SetTrigger(C_TRIGGER_TYPE_REGION_IMAGE, 0, C_TRIGGER_BEHAVIOR_PULSE, 2000);
	
end


function OnTriggerEnter(Ent)

 	
end

function OnTriggerExit(Ent)

	//LogMsg("Leaving region");
end

function OnTriggerInside(Ent)

	//kill any old voices
	PlayNoOverlap(this:Data():Get("File"));
 //don't let this one play again
 this:SetTrigger(C_TRIGGER_TYPE_NONE, 0, 0, 0);

end