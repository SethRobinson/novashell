
function OnInit() //run upon initialization
	
	this:Data():SetIfNull("Script", 'LogMsg("Hello!");');
	this:Data():SetIfNull("DeleteAfterRun", "false");
	
	if (toboolean(this:Data():Get("DeleteAfterRun")) == false or GameIsActive()) then
		this:SetVisibilityNotifications(true); //OnGetVisible and OnLoseVisible will get called now
	end
end

function OnPostInit() //run during the entity's first logic update
end

function OnGetVisible() //we are on the screen, run our script
	  	
 	ScheduleSystem(0, C_ENTITY_NONE, this:Data():Get("Script"));

	if (toboolean(this:Data():Get("DeleteAfterRun"))) then
				this:SetDeleteFlag(true);
	end
end

function OnLoseVisible()
end


function OnKill() //run when removed

end

