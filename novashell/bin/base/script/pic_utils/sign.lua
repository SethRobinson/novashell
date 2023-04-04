//text is displayed over the thing that is examined

function OnInit()
	this:Data():SetIfNull("OnActionText", "'Your ad here'");
	this:Data():SetIfNull("TextColor", "220 186 142 255");
	this:SetDefaultTalkColor(StringToColor(this:Data():Get("TextColor")));
end

function OnAction(enemy)

		GetTextManager:Add(this:Data():Get("OnActionText"), this);
end



