//load our intro menu code
LogMsg("Initting mod");

//GetApp:SetScreenSize(1024, 768); //if we wanted to force the screen res
GetGameLogic:InitGameGUI("game_gui/standard.xml");
RunScript("utils/rpg_utils.lua");  //helpful global misc functions that are game specific
RunScript("utils/audio_utils.lua"); //helpful global audio related functions that are game specific
RunScript("intro/intro_menu.lua");
RunScript("gui/interface_utils.lua"); //from base
RunScript("gui/items.lua");
RunScript("gui/player_menu.lua");
RunScript("gui/store_buy.lua");

//if 0 == 1 then
if g_isDebug then

		LogMsg("Debug mode detected");
		//skip the title screen, we're in debug mode and just want to edit
		//GetMapManager:SetActiveMapByName("Top View Inside");
		//GetGameLogic:ToggleEditMode();
		
		//Let's just load the active saved game, saves time during testing
		activeMenu = IntroMenu();
		activeMenu:StartGame();
	

	else
		//let's startup the intro menu
		activeMenu = IntroMenu();
		g_SmartSound:SetMusic("audio/ambience/cool_loop.ogg");
	end
	
//deal with what happens when escape is hit
GetInputManager:AddBinding("escape", "OnKeyEscape", C_ENTITY_NONE);	
	
	
function OnKeyEscape(bKeyDown)

  if (bKeyDown) then
	
	if (GetSpecialEntityByName("ChoiceDialog")) then
		return OnDialogKeyEscape(bKeyDown);
	end
	
		if (GetGameLogic:UserProfileActive()) then
		
				//they are currently playing the game...
				PlayDialogOpenSound();
				
				//is now a good time to open the menu?
				
			if ( GetPlayer:GetGoalManager():GetGoalCount() != 0 or GetPlayer:RunFunction("IsDead")) then
					//a cut scene must be active.. don't let them save now!
					entChoice = CreateEntitySpecial("ChoiceDialog", "msg=You can't open this menu while a cutscene is in progress.\n\nHold down TAB to make the active cutscene finish faster.;callback=OnPlayerMenuResponse;title=Wait!");
					entChoice:Send("add_choice|Continue|continue"); 
					entChoice:Send("activate");
					return false;  //don't process any other callbacks for this keypress
				
			end
				
				
				return OnMainMenu();  //defined in gui/player_menu.lua
			
			else
				//they are sitting at the main menu I guess
				return DefaultEscapeHandler(); //defined in base's misc_utils.lua
			end
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
	
	//turn off the mouse cursor
	GetApp:SetCursorVisible(false);
	
	InterfaceInit("gui/interface.lua");
	
end


	