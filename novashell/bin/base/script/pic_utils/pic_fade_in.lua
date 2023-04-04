function OnInit() //run upon initialization

	this:Data():SetIfNull("Fade Speed MS", "1000");

end

function OnPostInit() //run during the entity's first logic update

	this:SetBaseColor(Color(255,255,255,0)); //start as invisible
	this:GetBrainManager():Add("ColorFade","fade_speed_ms=" .. this:Data():Get("Fade Speed MS") .. ";remove_brain_when_done=true;a=255");
	
end

function OnKill() //run when removed

end

