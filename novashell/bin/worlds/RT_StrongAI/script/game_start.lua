//load our intro menu code

GetGameLogic:InitGameGUI("game_gui/standard.xml");
	
//deal with what happens when escape is hit
GetInputManager:AddBinding("escape", "OnKeyEscape", C_ENTITY_NONE);

function OnKeyEscape(bKeyDown)
  if (bKeyDown) then
	
		//return DefaultEscapeHandler(); //defined in base's misc_utils.lua
		LogMsg("Got escape");
		
		if (GetMapManager:GetActiveMap() == nil) then
			//the keyboard controls message must have kicked in
			return false;
			
		end
		
		
		if (GetMapManager:GetActiveMap():GetName() != "Intro") then
		//they are currently playing, let's quit back to the main menu
		GetGameLogic:SetRestartEngineFlag(true);
		else
			//escape from the intro menu should quit
				//	GetGameLogic:Quit(); //this will just quit the whole game
				
					return DefaultEscapeHandler(); //defined in base's misc_utils.lua
	
		end
		
	end

	return false; //don't process any other callbacks for this keypress
end

RunScript("main_menu.lua");
StartMainMenu();
