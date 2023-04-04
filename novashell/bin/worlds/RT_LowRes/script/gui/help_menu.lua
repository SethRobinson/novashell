//be careful, g_MenuButtons is 1 based (damn you lua), but g_curSelection is 0 based.

g_HelpMenuButtons = {"help_Back"}; //these match the names of the bottom images in
g_curHelpSelection = 0;

//ask the player if they really want to quit the game
function InitHelpMenu()
	
	//Unload and load the help menu to make sure it's 'fresh'
	
	GetMapManager:UnloadMapByName("Help"); //unload if it was loaded
	GetMapManager:SetActiveMapByName("Help"); //load and make active
	
	GetCamera:Reset(); //undo any weird lerp or scale settings we had
  SetGameVideoMode();
  GetCamera:SetPos(Vector2(0,0)); //set camera position of upper left corner in world coordinates
 
 	//setup input so we know when they hit a key or pushed up/down on the joystick
 	GetInputManager:AddBinding("control", "OnHelpMenuSelect", C_ENTITY_NONE);	
  GetInputManager:AddBinding("return", "OnHelpMenuSelect", C_ENTITY_NONE);	
  GetInputManager:AddBinding("space", "OnHelpMenuSelect", C_ENTITY_NONE);	
  GetInputManager:AddBinding("joy_any_button_0", "OnHelpMenuSelect", C_ENTITY_NONE);	

 	//the up down movement
	GetInputManager:AddBinding("up, always", "OnHelpMenuUp", C_ENTITY_NONE);	
	GetInputManager:AddBinding("down, always", "OnHelpMenuDown", C_ENTITY_NONE);	
	GetInputManager:AddBinding("joy_any_up,always", "OnHelpMenuUp", C_ENTITY_NONE);
	GetInputManager:AddBinding("joy_any_down,always", "OnHelpMenuDown", C_ENTITY_NONE);
	
	HelpUpdateVisuals(g_HelpMenuButtons[g_curHelpSelection+1]);
 
end

function KillHelpMenu()
  
  //remove the input we were listening for
  
  GetInputManager:RemoveBinding("control", "OnHelpMenuSelect", C_ENTITY_NONE);	
  GetInputManager:RemoveBinding("return", "OnHelpMenuSelect", C_ENTITY_NONE);	
  GetInputManager:RemoveBinding("space", "OnHelpMenuSelect", C_ENTITY_NONE);	
  GetInputManager:RemoveBinding("joy_any_button_0", "OnHelpMenuSelect", C_ENTITY_NONE);	

	//the up down movement
	GetInputManager:RemoveBinding("up, always", "OnHelpMenuUp", C_ENTITY_NONE);	
	GetInputManager:RemoveBinding("down, always", "OnHelpMenuDown", C_ENTITY_NONE);	
	GetInputManager:RemoveBinding("joy_any_up,always", "OnHelpMenuUp", C_ENTITY_NONE);
	GetInputManager:RemoveBinding("joy_any_down,always", "OnHelpMenuDown", C_ENTITY_NONE);

end

function OnHelpMenuSelect(bKeyDown)
	if (bKeyDown) then
			GetSoundManager:Play("audio/menu_choose.ogg");
			LogMsg("They pushed select");
			ProcessHelpMenuOption(g_HelpMenuButtons[g_curHelpSelection+1]);
		end

	return false;  //continue to process key callbacks for this key stroke?
end

function OnHelpMenuUp(bKeyDown)
	if (bKeyDown) then
			HelpMoveSelection(-1);
		end
	return false;  //continue to process key callbacks for this key stroke?
end

function OnHelpMenuDown(bKeyDown)
	if (bKeyDown) then
			HelpMoveSelection(1);
		end
	return false;  //continue to process key callbacks for this key stroke?
end

function HelpMoveSelection(amount)
	
	g_curHelpSelection = (g_curHelpSelection + amount) % #g_HelpMenuButtons;  //0 based
	LogMsg("Current option selected is", g_MenuButtons[g_curSelection+1]);
	
	GetSoundManager:Play("audio/menu_select.ogg");
	HelpUpdateVisuals(g_HelpMenuButtons[g_curHelpSelection+1]);
end


function HelpUpdateVisuals(entName)

	//let's find this entity, make sure it's on the current map, and highlight it somehow
	
	 local selectionEnt = GetEntityByName(entName);
	 if (selectionEnt == nil) then
	 	LogError("No entity named ", entName, "on the Help menu.");
	 end

	local highlighterEnt = GetEntityByName("highlighter");
	
	if (highlighterEnt == nil) then
		LogMsg("Creatin ghighligher..");
		//that's ok, we just haven't created it yet
		highlighterEnt = GetEntityByName("highlighter_template"):Clone(GetMapManager:GetActiveMap(), Vector2(0,0));
		highlighterEnt:SetPersistent(false); //won't save
		highlighterEnt:SetName("highlighter"); //so we can't find it later
	end
	
		//make sure it's on our map
		highlighterEnt:SetPosAndMap(Vector2(0,0), GetMapManager:GetActiveMap():GetName());
		//GetWatchManager:Add(highlighterEnt, 1000);
	 //move the highlighter the the right position so it matches the selection
	 
	local selectionRect = selectionEnt:GetWorldRect();
	local vPos = Vector2(selectionRect.left, selectionRect.top + selectionEnt:GetSizeY()/2);
	vPos.x = vPos.x - 10; //move it over a bit to the left, looks better
	highlighterEnt:SetPos(vPos);
	LogMsg("Setting highlighter to", vPos);

end

function ProcessHelpMenuOption(optionName)

	LogMsg("Player chose the entity named", optionName);

	KillHelpMenu();
	
	if (optionName == "help_Back") then
	
		InitMainMenu();
		return;
	end
	
	LogError("Don't know how to handle option", optionName);

end
