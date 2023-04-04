function OnInit()
	this:SetVisualProfile("~/misc.xml", "ent_apple");
	this:LoadCollisionInfo("~/ent_apple.col");
end


function OnPostInit()
	this:SetDensity(0.2);
	this:SetEnableRotationPhysics(true);
end


function OnAction(enemy)
	GetTextManager:Add("It's an apple.", enemy);
end

