function OnInit()
	
	this:Data():SetIfNull("OnActionText", "I don't know what this is.");
end

function OnAction(Ent)
	GetTextManager:Add(this:Data():Get("OnActionText"), Ent);
end
