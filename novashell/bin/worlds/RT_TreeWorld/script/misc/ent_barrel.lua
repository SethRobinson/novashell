function OnInit()
	this:SetVisualProfile("~/misc.xml", "ent_barrel");
	this:LoadCollisionInfo("~/ent_barrel.col");
	this:SetEnableRotationPhysics(true);
end

function OnPostInit()
end

function OnAction(enemy)
	GetTextManager:Add("It's a heavy looking barrel.", enemy);
end

