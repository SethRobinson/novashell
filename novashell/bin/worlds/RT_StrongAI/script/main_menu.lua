_G.g_sfxID = nil;
_G.g_musicID = nil;


function PlayNoOverlap(soundFile)
	if (_G.g_sfxID != nil) then
		GetSoundManager:Kill(_G.g_sfxID);
	end
 _G.g_sfxID = GetSoundManager:Play(soundFile);
end


function PlayMusicNoOverlap(soundFile)
	
	if (g_musicEnabled) then
		if (_G.g_musicID != nil) then
			GetSoundManager:Kill(_G.g_musicID);
		end
	 _G.g_musicID = GetSoundManager:PlayLooping(soundFile);
	end
end


function StartMainMenu()

	if (GetInputManager:GetJoystickCount() == 0) then
		if (VariableExists("bFirstTime") == false) then
			bFirstTime = false; //don't show this again
		
		ShowMessage("No joystick/gamepad detected",
		"This game was designed for a dual-stick analog controller such as the XBOX 360 controller.\n\nIf you've got one, plug it in and restart!\n\nIf not, you can use W,A,S,D to move, the arrow keys to shoot, and left CTRL to start the game.\n\nClick OK to continue", true);
	
		end
	end

	
	GetMapManager:UnloadMapByName("Main");
	GetMapManager:SetActiveMapByName("Intro");
	GetCamera:Reset(); //undo any weird lerp or scale settings we had
  
  GetInputManager:AddBinding("control", "OnGameStart", C_ENTITY_NONE);	
  GetInputManager:AddBinding("return", "OnGameStart", C_ENTITY_NONE);	
  GetInputManager:AddBinding("space", "OnGameStart", C_ENTITY_NONE);	
  GetInputManager:AddBinding("joy_any_button_0", "OnGameStart", C_ENTITY_NONE);	

	PlayMusicNoOverlap("audio/ambience.ogg");
	g_timeStart = GetApp:GetGameTick(); //so we know when they win, we can show the time

end

function KillMainMenu()

	//unbind what we had bound
	
  GetInputManager:RemoveBinding("control", "OnGameStart", C_ENTITY_NONE);	
  GetInputManager:RemoveBinding("return", "OnGameStart", C_ENTITY_NONE);	
  GetInputManager:RemoveBinding("space", "OnGameStart", C_ENTITY_NONE);	
  GetInputManager:RemoveBinding("joy_any_button_0", "OnGameStart", C_ENTITY_NONE);	

end

function OnGameStart(bKeyDown)
	if (bKeyDown) then
		KillMainMenu();
		GetMapManager:SetActiveMapByName("Main");
		GetCamera:Reset(); //undo any weird lerp or scale settings we had
		local vStartPos = GetTagManager:GetPosFromName("start_pos"); //find the entity called this, get its position
		GetCamera:SetPosCentered(vStartPos);
		local player = GetEntityByName("player_template"):Clone(GetMapManager:GetActiveMap(), vStartPos);
		player:RunFunction("Init");		
		GetCamera:SetEntityTrackingOffset(Vector2(0,0)); //center it exactly in the middle
		GetApp:SetCursorVisible(false); //so we can't see the mouse
		
		if (C_SCREEN_X < 1200) then
			//zoom out a bit to see more of the action
		GetCamera:SetScaleTarget(Vector2(0.8,0.8));
		end
		
		g_timeStart = GetApp:GetGameTick(); //so we know when they win, we can show the time
	end
	
	
	
	return true;
end
