function OnInit()
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

	local total_seconds = (GetApp:GetGameTick() - g_timeStart) / 1000;
	total_seconds = math.floor(total_seconds);
	local min = math.floor(total_seconds/60);
	local seconds = total_seconds - (min * 60);
	local text = "Speed run: " .. string.format("%.0f", min)  .. ":" .. string.format("%.0f", seconds);


	//build a text object to show this here
	
		local entText = GetEntityByName("text_template"):Clone(GetMapManager:GetActiveMap(), this:GetPos());
		entText:RunFunction("SetText", text);
		entText:RunFunction("SetTextScale", Vector2(2,2));
		entText:GetBrainManager():Add("FadeIn", "fade_speed_ms=300");		
		entText:SetPersistent(false);
	
	//don't let this happen again
 this:SetTrigger(C_TRIGGER_TYPE_NONE, 0, 0, 0);

end