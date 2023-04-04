function OnInit()
	
	this:SetVisualProfile("~/misc.xml", "ent_crate");
	this:SetDensity(0.4);
	this:LoadCollisionInfo("~/ent_crate.col");
	this:SetEnableRotationPhysics(true);
end

function OnPostInit()
	this:SetVisibilityNotifications(true); //OnGetVisible and OnLoseVisible will get called now
end

function OnGetVisible()
end

function OnLoseVisible() //we left the screen

	//let this entity function offscreen for a while
	GetWatchManager:Add(this, 3000); //will call OnWatchTimeout(bIsOnScreen) when timed out if it exists
end

function OnAction(enemy)

	GetTextManager:Add("It's a crate.", enemy);
   
end

