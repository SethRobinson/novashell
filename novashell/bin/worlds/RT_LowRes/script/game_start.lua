//load our intro menu code
LogMsg("Initting world");


function SetGameVideoMode()

//we want to use graphics designed for 320X240, but we're playing in 640X480, so let's zoom in a bit
	GetApp:SetScreenSize(640,480);
	GetCamera:SetScale(Vector2(2,2)); //2x as big
	GetCamera:SetMoveLerp(0.1); //slow
	GetCamera:SetScaleLerp(0.1);
	GetCamera:InstantUpdate(); //don't let it lerp slowly
//note, in our overidden system/editor.lua, we call this when the editor is closed
end

SetGameVideoMode(); 

GetGameLogic:InitGameGUI("game_gui/standard.xml");
RunScript("gui/interface_utils.lua"); //from base

GetApp:SetCursorVisible(false); //so we can't see the mouse
	
	//deal with what happens when escape is hit
GetInputManager:AddBinding("escape", "OnKeyEscape", C_ENTITY_NONE);
	
function OnKeyEscape(bKeyDown)

  if (bKeyDown) then
	
		if (GetMapManager:GetActiveMap():GetName() == "Main Menu") then
			//on the main menu, let the default handler quit it out to the world select menu
			return DefaultEscapeHandler(); //defined in base's misc_utils.lua
		else
			//quit the active game, go back to the main menu

			//don't let old map stuff hang around
			GetMapManager:UnloadMapByName(GetMapManager:GetActiveMap():GetName()); 

			InitMainMenu(); //get our custom menu going, loads the main menu map

		end
	end

	return false; //don't process any other callbacks for this keypress
	  
end

//start up the intro menu

RunScript("gui/main_menu.lua");

InitMainMenu(); //get our custom menu going, loads the main menu map

function OnIntroMenuStart()

	LogMsg("They clicked start, we're called by the code in intro/intro_menu.lua");
	g_totalCoinsPlaced = -1; //-1 because the coin on the interface adds itself..
	InterfaceInit("gui/interface.lua");

end

//these will over-ride the editor.lua functions in base to add our special screen size
//changes when the editor is turned on/off


function OnOpenEditor()
	LogMsg("Custom OnOpenEditor function...");
	g_SmartSound:FadeOutCurrentMusic();
	g_SmartAmbience:FadeOutCurrentMusic();
	
	SetRetailMode(false); //if we're opening the editor, we need to disable retail mode
	//if it wasn't already, so the F1 toggle will become active so we can close it
	
	
	g_editorActive = true;
	GetSoundManager:Play("audio/system/editor.ogg");
	GetApp:SetScreenSize(1024,768);
	g_bLastCursorOn = GetApp:GetCursorVisible();
	GetApp:SetCursorVisible(true);
end



function OnCloseEditor()
	GetApp:SetCursorVisible(g_bLastCursorOn);
	g_editorActive = false;
	
	if (GetMapManager:GetActiveMap() == nil) then
		//they probably quit the game from the editor
		return;

	end	
	
	SetGameVideoMode(); //return the screensize and zoom to the games setting, I put this func in game_startup.lua
	//rebuild the visual map area in case it has changed
	
	r = GetMapManager:GetActiveMap():ComputeWorldRect(0); //the parm is reserved, so we use 0 for now
	LogMsg("Map used area is ", r);
		
	LogMsg("Editor closed");
	
	CenterCameraOnThisEntityIfAvailable("Player"); //a scripted function in base
end




	
	