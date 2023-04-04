function OnInit()

	this:SetImage("rpg_tiny_arrow.png", nil); //nil means use the image size, no clip rect
	this:SetAlignment(C_ORIGIN_CENTER, Vector2(0,0));
end

function OnPostInit()
	this:GetBrainManager():Add("Bob","speed=1;distance=2;visual_interval_ms=0");

end