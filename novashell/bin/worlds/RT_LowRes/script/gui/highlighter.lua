
function OnInit() //run upon initialization

	this:SetVisualProfile("char/player/player.xml", "player");
	this:LoadCollisionInfo("char/player/player.col");
	
end

function OnPostInit() //run during the entity's first logic update
end

function OnKill() //run when removed

end

