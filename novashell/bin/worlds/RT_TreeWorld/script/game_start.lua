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

		//get sound music going
  	    g_SmartSound:SetMusic("audio/ambience/cool_loop.ogg");

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

	LogMsg("Running our custom tree world game start code.");
	g_totalCoinsPlaced = -1; //-1 because the coin on the interface adds itself..
	
	LogMsg("Setting game defaults");


	GetMapManager:SetActiveMapByName("Inside"); //cause this to get preloaded
	GetMapManager:SetActiveMapByName("Main"); //cause this to get preloaded

	InterfaceInit("gui/interface.lua");
	
	
	//GetCamera:SetLimitToMapArea(true); //try not to show blank parts of the map
	//r = GetMapManager:GetActiveMap():ComputeWorldRect(0); //the parm is reserved, so we use 0 for now
	//LogMsg("Map used area is ", r);
	
end

	