RunScript("game_loop.lua");
RunScript("gui/help_menu.lua");

//be careful, g_MenuButtons is 1 based (damn you lua), but g_curSelection is 0 based.
g_MenuButtons = {"main_Play1", "main_Play2","main_Play3","main_Play4", "main_Help", "main_Edit", "main_Quit"}; //these match the names of the bottom images in
g_curSelection = 0;
g_curLevel = 1;
g_numPlayers = 0;
g_PlayID = {}; //will hold the entity id's of our players

C_LEVEL_WON = 1000;

//ask the player if they really want to quit the game
function InitMainMenu()
  //get sound music going
	//g_SmartSound:SetMusic("audio/ambience/cool_loop.ogg");
	GetCamera:SetLimitToMapArea(false); //try not to show blank parts of the map
	
	//Unload and load the main menu to make sure it's 'fresh'
	
	GetMapManager:UnloadMapByName("Main Menu"); //unload if it was loaded
	GetMapManager:SetActiveMapByName("Main Menu"); //load and make active
	
	GetCamera:Reset(); //undo any weird lerp or scale settings we had
  SetGameVideoMode();
  GetCamera:SetPos(Vector2(0,0)); //set camera position of upper left corner in world coordinates
 
 	//setup input so we know when they hit a key or pushed up/down on the joystick
 	GetInputManager:AddBinding("control", "OnMainMenuSelect", C_ENTITY_NONE);	
  GetInputManager:AddBinding("return", "OnMainMenuSelect", C_ENTITY_NONE);	
  GetInputManager:AddBinding("space", "OnMainMenuSelect", C_ENTITY_NONE);	
  GetInputManager:AddBinding("joy_any_button_0", "OnMainMenuSelect", C_ENTITY_NONE);	

 	//the up down movement
	GetInputManager:AddBinding("up, always", "OnMainMenuUp", C_ENTITY_NONE);	
	GetInputManager:AddBinding("down, always", "OnMainMenuDown", C_ENTITY_NONE);	
	GetInputManager:AddBinding("joy_any_up,always", "OnMainMenuUp", C_ENTITY_NONE);
	GetInputManager:AddBinding("joy_any_down,always", "OnMainMenuDown", C_ENTITY_NONE);
	g_numPlayers = 0; //important, so in game_loop.lua we know not to process the camera
	
	MainUpdateVisuals(g_MenuButtons[g_curSelection+1]);
 
 
end

function KillMainMenu()
  
  //remove the input we were listening for
  
  GetInputManager:RemoveBinding("control", "OnMainMenuSelect", C_ENTITY_NONE);	
  GetInputManager:RemoveBinding("return", "OnMainMenuSelect", C_ENTITY_NONE);	
  GetInputManager:RemoveBinding("space", "OnMainMenuSelect", C_ENTITY_NONE);	
  GetInputManager:RemoveBinding("joy_any_button_0", "OnMainMenuSelect", C_ENTITY_NONE);	

	//the up down movement
	GetInputManager:RemoveBinding("up, always", "OnMainMenuUp", C_ENTITY_NONE);	
	GetInputManager:RemoveBinding("down, always", "OnMainMenuDown", C_ENTITY_NONE);	
	GetInputManager:RemoveBinding("joy_any_up,always", "OnMainMenuUp", C_ENTITY_NONE);
	GetInputManager:RemoveBinding("joy_any_down,always", "OnMainMenuDown", C_ENTITY_NONE);


end

function OnMainMenuSelect(bKeyDown)
	if (bKeyDown) then
			GetSoundManager:Play("audio/menu_choose.ogg");
			LogMsg("They pushed select");
			ProcessMainMenuOption(g_MenuButtons[g_curSelection+1]);
		end

	return false;  //continue to process key callbacks for this key stroke?
end

function OnMainMenuUp(bKeyDown)
	if (bKeyDown) then
			MainMoveSelection(-1);
		end
	return false;  //continue to process key callbacks for this key stroke?
end

function OnMainMenuDown(bKeyDown)
	if (bKeyDown) then
			MainMoveSelection(1);
		end
	return false;  //continue to process key callbacks for this key stroke?
end

function MainMoveSelection(amount)
	g_curSelection = (g_curSelection + amount) % #g_MenuButtons;  //0 based
	LogMsg("Current option selected is", g_MenuButtons[g_curSelection+1]);
	
	GetSoundManager:Play("audio/menu_select.ogg");
	MainUpdateVisuals(g_MenuButtons[g_curSelection+1]);
end


function MainUpdateVisuals(entName)

	//let's find this entity, make sure it's on the current map, and highlight it somehow
	
	 local selectionEnt = GetEntityByName(entName);
	 if (selectionEnt == nil) then
	 	LogError("No entity named ", entName, "on the main menu.");
	 end

	local highlighterEnt = GetEntityByName("highlighter");
	
	if (highlighterEnt == nil) then
		//that's ok, we just haven't created it yet
		highlighterEnt = GetEntityByName("highlighter_template"):Clone(GetMapManager:GetActiveMap(), Vector2(0,0));
		highlighterEnt:SetPersistent(false); //won't save
		highlighterEnt:SetName("highlighter"); //so we can't find it later
	end
	
		//make sure it's on our map
		highlighterEnt:SetPosAndMap(Vector2(0,0), GetMapManager:GetActiveMap():GetName());

	 //move the highlighter the the right position so it matches the selection
	 
	 local selectionRect = selectionEnt:GetWorldRect();
	 local vPos = Vector2(selectionRect.left, selectionRect.top + selectionEnt:GetSizeY()/2);
	 vPos.x = vPos.x - 10; //move it over a bit to the left, looks better
	 highlighterEnt:SetPos(vPos);
	 LogMsg("Setting highlighter to", vPos);

end

function ProcessMainMenuOption(optionName)

	LogMsg("Player chose the entity named", optionName);

	KillMainMenu();
	
	if (optionName == "main_Quit") then
	
		//go back to the world select menu
		GetGameLogic:ClearModPaths(); //so this world won't load again
		GetGameLogic:SetRestartEngineFlag(true);
	  GetApp:SetScreenSize(1024, 768); //put the screensize back to normal
		return;
	end
	
	if (optionName == "main_Play1") then
			g_numPlayers = 1;
			SetupLevel(1);
			return;
		end
		
	if (optionName == "main_Play2") then
			g_numPlayers = 2;
			SetupLevel(1);
			return;
		end

	if (optionName == "main_Play3") then
			g_numPlayers = 3;
			SetupLevel(1);
			return;
		end

	if (optionName == "main_Play4") then
			g_numPlayers = 4;
			SetupLevel(1);
			return;
		end
		
		if (optionName == "main_Help") then
			InitHelpMenu();
			return;
		end

	if (optionName == "main_Edit") then
			ShowMessage("Edit Mode", "You can now edit the map files.\n\nChanges will be automatically saved.\n\nUse F1 to toggle the editor.", true);
			GetMapManager:SetActiveMapByName("Level 1"); //default map to start editing on
			GetGameLogic:ToggleEditMode();
		return;
	end
	
	
	LogError("Don't know how to handle option", optionName);

end

function SetupLevel(levelNum)

	g_curLevel = levelNum;

	//look for a player start marker of a specific name.. the map name doesn't matter, we'll find it
	local markerName = "player_" .. levelNum .. "_start";
	local startEnt = GetEntityByName(markerName);
	
	if (startEnt == nil) then
				
				//well, there is no level with this name, so let's assume they won the game and show our won game
				//screen.
				
				markerName = "player_won_start";
				startEnt = GetEntityByName(markerName);
				g_curLevel = C_LEVEL_WON;
				if (startEnt == nil) then
					LogError("There should be an entity named", markerName, "somewhere. This marks the player start position.");
					return;
				end
			end

		GetMapManager:SetActiveMapByName(startEnt:GetMap():GetName());
		GetCamera:SetLimitToMapArea(true); //try not to show blank parts of the map
		
		LogMsg("Cloning player..");
		
		for i=0, g_numPlayers-1, 1 do

		local newPlayer = GetEntityByName("Player_template"):Clone(startEnt:GetMap(), startEnt:GetPos());
			newPlayer:SetPersistent(false); //don't let it save with the map
			newPlayer:RunFunction("InitPlayer", i);
			g_PlayID[i] = newPlayer:GetID(); //for quick access later in our game loop
			GetCamera:SetPosCentered(newPlayer:GetPos());
			
		end
		
		SetGameVideoMode(); //override whatever the map camera was set at, zoom-wise
end