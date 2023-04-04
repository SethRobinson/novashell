//load our intro menu code
LogMsg("Initting world");

//app specific lua code we want to include now so they'll be system wide
RunScript("main_menu.lua");
RunScript("game.lua");

//GetApp:SetScreenSize(800, 600); //if we wanted to force the screen res
GetGameLogic:InitGameGUI("game_gui/standard.xml");

//SetupMainMenu();
SetupGame();
	
//deal with what happens when escape is hit
GetInputManager:AddBinding("escape", "OnKeyEscape", C_ENTITY_NONE);

function OnKeyEscape(bKeyDown)

  if (bKeyDown) then
	
		return DefaultEscapeHandler(); //defined in base's misc_utils.lua
	end

	return false; //don't process any other callbacks for this keypress
end


