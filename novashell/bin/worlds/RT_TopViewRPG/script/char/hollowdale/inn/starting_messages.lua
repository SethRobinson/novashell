//I use this with a "run line of script" helper, just setup the helper to run:
//RunScript("char/hollowdale/starting_messages.lua");
	
		LogMsg("Running starting_messages.lua..");
	
		//let's do a little cutscene of him waking up, illustrates how we can take control of a
		//character and have him do things
		
		//uncomment below to show a little startup cutscene
		--[[
	    local goal = GetPlayer:GetGoalManager():PushNewGoal("RiseAndShine");
	  	goal:AddDelay(1000); //wait a second before starting
	  	goal:AddSay("<yawn>", C_FACING_NONE);
	   	goal:AddApproach(GetEntityByName("Hol_Inn_PlayerRoom"):GetID(), C_DISTANCE_CLOSE);
	   	goal:AddSay("Another boring day.  Guess I better go harass the maid.", C_FACING_NONE, 0); //the zero is the optional MS time to wait for the text
		  goal:AddRunScriptString("ResetKeys();");
		--]]

local function AddText(message, timeMS)

		Schedule(timeMS, C_ENTITY_NONE,
			'GetTextManager:AddCustomScreen("' .. message .. '", Vector2(C_SCREEN_X/2, C_SCREEN_Y-50), Vector2(0, 0),  Color(200,200,0,255), 4000, C_FONT_DEFAULT);');
end
		
		local scheduleTimeMS = 500; //show the first message at this time
		AddText("Use the arrow keys to move.", scheduleTimeMS);
		
		scheduleTimeMS = scheduleTimeMS + 5000;
		AddText("Press SPACE to examine the object you are facing.", scheduleTimeMS);

		scheduleTimeMS = scheduleTimeMS + 5000;
		AddText("Press CTRL to attack.", scheduleTimeMS);

	  scheduleTimeMS = scheduleTimeMS + 5000;
		AddText("Hold TAB to speedup the game.", scheduleTimeMS);

    scheduleTimeMS = scheduleTimeMS + 5000;
		AddText("F1 to toggle edit mode.", scheduleTimeMS);
