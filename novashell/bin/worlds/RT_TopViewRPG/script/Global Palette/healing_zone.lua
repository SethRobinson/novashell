//assign to an invisible pic, this will heal the player while he's standing in the area


function OnInit() //run upon initialization
	this:Data():SetIfNull("healAmount", "2");

	if (GameIsActive()) then
		//this is a real game, not the editor
		//this trigger will call OnTriggerInside every 2000 MS, plus the enter and exit callbacks
		this:SetTrigger(C_TRIGGER_TYPE_REGION_IMAGE, 0, C_TRIGGER_BEHAVIOR_PULSE, 1000);
	end


end

function OnTriggerEnter(Ent)
	//don't care
end

function OnTriggerExit(Ent)
	//don't care
end

function OnTriggerInside(Ent)
		GetPlayer:RunFunction("OnHeal", this:Data():GetNum("healAmount"));
end