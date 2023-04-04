//note:  This is meant to be overriden by the mod

//if this is run likely no mod was specified, let's guess.

//to override this from the command prompt, run game.exe worldname.novashell


LogMsg("No mod has overidden script/game_start.lua.  Running default, which let's the player choose a world.");

GetGameLogic:ClearModPaths();
GetGameLogic:InitGameGUI("game_gui/standard.xml"); //load fonts/gui styles we'll use for menus
GetMapManager:SetActiveMapByName("hidden_Base Startup");

//give the keyboard a chance to flush itself out before the world dialog is created
Schedule(270, C_ENTITY_NONE, 'chooseMenuID = CreateEntitySpecial("WorldChooseDialog", "")');


//to automatically load a world without asking, comment out the above CreateEntitySpecial and uncomment this
//GetGameLogic:AddModPath("Test World");
//GetGameLogic:SetRestartEngineFlag(true);	

//deal with what happens when escape is hit
GetInputManager:AddBinding("escape", "OnKeyEscape", C_ENTITY_NONE);


function OnKeyEscape(bKeyDown)

  if (bKeyDown) then
	
	if (GetSpecialEntityByName("ChoiceDialog")) then
		return OnDialogKeyEscape(bKeyDown);
	end
	
	  if (GameIsActive()) then
	   //they are currently playing, let's quit back to the main menu?
	
	   GetGameLogic:SetRestartEngineFlag(true);
	   else
			//they are sitting at the main menu or editing stuff
			GetGameLogic:Quit(); //this will just quit the whole game
	   end
	 end
	  
	  return false; //don't process any other callbacks for this keypress
end
