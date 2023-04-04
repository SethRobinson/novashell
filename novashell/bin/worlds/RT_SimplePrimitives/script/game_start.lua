//Mega simple script to startup our main loop and install the escape key to quit

//deal with what happens when escape is hit
GetInputManager:AddBinding("escape", "OnKeyEscape", C_ENTITY_NONE);
	
function OnKeyEscape(bKeyDown)

  if (bKeyDown) then
			return DefaultEscapeHandler(); //defined in base's misc_utils.lua
	end
	
	return true; //keep processing other callbacks for this key if applicable
end

GetMapManager:SetActiveMapByName("Main"); 

//run our main game loop
RunScript("game_loop.lua");
	