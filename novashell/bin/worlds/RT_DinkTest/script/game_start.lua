//load our intro menu code
LogMsg("Initting world");

//GetApp:SetScreenSize(800, 600); //if we wanted to force the screen res
GetGameLogic:InitGameGUI("game_gui/standard.xml");
RunScript("intro/intro_menu.lua");
RunScript("gui/interface_utils.lua"); //from base


if 0 == 1 then
//if g_isDebug then

		GetMapManager:SetActiveMapByName("Top View Inside");
		GetGameLogic:ToggleEditMode();

	else

		//let's startup the intro menu
		activeMenu = IntroMenu();
	end
	
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
	InterfaceInit("gui/interface.lua"); //get the interface drawing
end


	