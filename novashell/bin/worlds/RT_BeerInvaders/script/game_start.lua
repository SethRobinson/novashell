//load our intro menu code
LogMsg("Initting world");

//GetApp:SetScreenSize(800, 600); //if we wanted a smaller screen..
GetGameLogic:InitGameGUI("game_gui/standard.xml");
RunScript("intro/intro_menu.lua");
RunScript("init_level.lua"); //our main invader game logic functions

//let's startup the default intro menu defined in intro_menu.lua.
activeMenu = IntroMenu();

//deal with what happens when escape is hit
GetInputManager:AddBinding("escape", "OnKeyEscape", C_ENTITY_NONE);


function OnKeyEscape(bKeyDown)

  if (bKeyDown) then
	
		return DefaultEscapeHandler(); //defined in base's misc_utils.lua
	end

	return false; //don't process any other callbacks for this keypress
end

function OnIntroMenuStart()
	//run when someone clicks new or continue from the intro screen, allows you to perform some
	//task before the map is loaded if needed. (called from intro_menu.lua)
end


function OnPostIntroMenuStart()
	
	//run after the intro menu has loaded/continued a new game, allows you do move the camera or change maps
	//or do anything. (called from intro_menu.lua)
	InitLevel(1); //start up level 1, this function is located in script/init_level.lua
end
