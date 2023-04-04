//a simple menu class system the examples use
//you should copy this to your own worlds folder and edit it for more control

IntroMenu = class(); //class helper defined in misc_utils.lua

function IntroMenu:__init() //the constructor, automatically called when object is initted

	GetApp:SetCursorVisible(true); //so we can see the mouse pointer
	LogMsg("Initting intro menu");
	GetMapManager:SetActiveMapByName("Intro");
	GetCamera:Reset(); //undo any weird lerp or scale settings we had
  
	//get notified when the left mouse button is clicked
	GetInputManager:AddBinding("mouse_left", "OnIntroLeftMouseClick", C_ENTITY_NONE);
	
	self.defaultProfileName = "Player";
	self.defaultMapName = "Main"; //when edit is clicked, we'll start with this map name.
	self.defaultPlayerName = "Player"; //we'll look for an entity with this name when starting a game
	
end

function IntroMenu:Kill()

	//remove the callback
	local success = GetInputManager:RemoveBinding("mouse_left", "OnIntroLeftMouseClick", C_ENTITY_NONE);
	//if you really cared, you could check success to see if it actually did it or not, it's a boolean

end

function IntroMenu:StartGame()

	//from this moment forth, all 'touched' map data will be saved here
	//note, slashes, backslashes and other special chars are stripped from input here
	GetGameLogic:SetUserProfileName(self.defaultProfileName); //this clears all maps from mem
	
	if (FunctionExists("OnIntroMenuStart")) then
		//if this exists anywhere, we'll run it.  Helps with having the examples setup the
		//game.  In a real game, you probably wouldn't be using this intro_menu.lua anyway.
		OnIntroMenuStart();
	end
	
	
	g_SmartSound:FadeOutCurrentMusic(); //just in case any is playing
   
	//find the player and move the camera to him
	local playerTag = GetTagManager:GetFromString(self.defaultPlayerName);
	
	if (playerTag == nil) then
		 ShowMessage("Error", "Can't locate an entity named " .. self.defaultPlayerName .. "  Note that it's case sensitive.", false);
		//let them continue anyway
      GetMapManager:SetActiveMapByName(self.defaultMapName);
      	return;
		 end
	 
      GetCamera:Reset(); //undo any weird lerp or scale settings we had
      local mapName = playerTag:GetMapName();

	  LogMsg("Found player in " .. mapName .. " at " .. tostring(playerTag:GetPos()));

      GetMapManager:SetActiveMapByName(mapName);
      GetCamera:SetScale(Vector2(1,1)); 
      
      GetCamera:SetPosCentered(playerTag:GetPos());
      GetCamera:InstantUpdate();
	  
	  if (FunctionExists("OnPostIntroMenuStart")) then
		//if this exists anywhere, we'll run it.  Helps with having the examples setup the
		//game.  In a real game, you probably wouldn't be using this intro_menu.lua anyway.
		OnPostIntroMenuStart();
	end
	
	  
	  self:Kill();

end

function IntroMenu:OnLeftMouseClick(bButtonDown)

	if (bButtonDown == true) then
		return true; //we only care when the button is released, not when it is pressed
	end
	//LogMsg("Got left mouse click: " .. tostring(pt));
	local pt = ScreenToWorld(GetInputManager:GetMousePos());
	local Ent = GetEntityByWorldPos(pt, nil, false); //that last false can be true for pixel accurate checks
	
	if (Ent == nil) then return true; end; //clicked nothing useful
	
    //note, this map has been marked "AutoSave off" in the editor, so we can mess it up like this
	
	if (Ent:GetMap():GetName() != GetMapManager:GetActiveMap():GetName()) then
		//uh, why is this script still running if we're not on the intro menu?
		//they are probably screwing around in the editor.  Just ignore this click.
		return true;
	end
	
	Ent:InitCollisionDataBySize(0,0); //0,0 means use image size.  this will give it physics and stuff, gravity will make it fall
	Ent:SetEnableRotationPhysics(true);
	
  //let's wait a bit before doing anything
	if (Ent:GetName() == "") then
	
	//it has no name, let's assume we don't care what it is
	else
  		//don't allow more selections
		self:Kill();
		ScheduleSystem(1000, C_ENTITY_NONE, "activeMenu:ProcessOption(\"" .. Ent:GetName() .. "\")");
	end
	  
	return true; //the true means continue to process other callbacks on this key that were added earlier than us
end

function IntroMenu:ProcessOption(optionText)

	
	if (optionText == "ButtonNew") then
   	    GetGameLogic:ResetUserProfile(self.defaultProfileName); //deletes save files here
	  	self:StartGame();

	  elseif (optionText == "ButtonContinue") then
  	    //let's use this data, if there isn't any, it's really the same as New
		    
		    if (GetGameLogic:UserProfileExists(self.defaultProfileName) == false) then
					ShowMessage("Can't continue", "No active game exists.  Choose New.", true);
					//GetGameLogic:SetRestartEngineFlag(true);
					
					//a way to reload the map
					local worldName = GetMapManager:GetActiveMap():GetName();
					GetMapManager:UnloadMapByName(worldName);
					GetMapManager:SetActiveMapByName(worldName);
	  
	  			    //turn the mouse back on
	  				GetInputManager:AddBinding("mouse_left", "OnIntroLeftMouseClick", C_ENTITY_NONE);
	
		    else
		  		  	self:StartGame();

		  	end

	  elseif (optionText == "ButtonEdit") then
		self:Kill();
		ShowMessage("Edit Mode", "You can now edit the base map files.\n\nChanges will be automatically saved.\n\nRestart game and choose New to see changes.\n\nUse F1 to toggle the editor.", true);
		GetCamera:Reset(); //undo any weird lerp or scale settings we had
		GetMapManager:SetActiveMapByName(self.defaultMapName); //default map to start editing on
		GetGameLogic:ToggleEditMode();
	  else
	  
	  //don't know what kind of button this is.  ignore it and get ready for another one
	
	  //turn the mouse back on
	  GetInputManager:AddBinding("mouse_left", "OnIntroLeftMouseClick", C_ENTITY_NONE);
	
	  end

end


//I would rather the callback be to the class instance directly but I don't know how to do that?
function OnIntroLeftMouseClick(bButtonDown)
			return activeMenu:OnLeftMouseClick(bButtonDown);
end


//dummy defaults, this can easily be overriden later by assigning a different function to this name

function OnIntroMenuStart()
	//run when the menu is initted	
end

function OnPostIntroMenuStart()
	//run after the game is started
end