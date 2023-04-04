LogMsg("Novashell V" .. GetApp:GetEngineVersionAsString() .. " and " .. _VERSION .. " Initted.");
RunScript("system/safety.lua"); //Ian's stuff to force "strict" var checking, uncomment for more speed
RunScript("system/setup_constants.lua"); //loads script.  checks all mounted filesystems in order of mounting (mods first, for instance)

g_isDebug = false; //doesn't really mean much, but your game could check for this and do something useful. (using GetApp:ParmExists("-debug"))

g_isRetail = false; //if true, editor/etc will be disabled
g_musicEnabled = true;

if (g_isRetail == false) then
	GetInputManager:AddBinding("f1,game_and_editor,always", "OnToggleEditMode", C_ENTITY_NONE);
end


function SetRetailMode(bRetail)

		LogMsg("Setting retail to", bRetail);
	
	if (g_isRetail == bRetail) then
		return;
	end;

	//status changed
	
	if (bRetail) then
		GetInputManager:RemoveBinding("f1,game_and_editor,always", "OnToggleEditMode", C_ENTITY_NONE);
		LogMsg("Retail mode activated.");
	else
			LogMsg("Retail mode disabled");
			GetInputManager:AddBinding("f1,game_and_editor,always", "OnToggleEditMode", C_ENTITY_NONE);
	end

	g_isRetail = bRetail;

end


	if (GetApp:ParmExists("-retail")) then
  	SetRetailMode(true);
	end
	
	if (GetApp:ParmExists("-debug")) then
  	g_isDebug = true;
  	SetRetailMode(false);
   	LogMsg("Debug mode activated.");
	end

if (GetApp:ParmExists("-nomusic")) then
  	g_musicEnabled = false;
   	LogMsg("Music and looping ambient sounds disabled.");
	end


RunScript("system/misc_utils.lua");
RunScript("system/joystick.lua");
RunScript("system/sound.lua");
RunScript("gui/dialog.lua"); //has the sound effects to use for the dialog, things like that

//include other files
RunScript("system/editor.lua");
RunScript("effects/effect_utils.lua");

//assign global keys.  Note, you could always use RemoveBinding() to remove them later from a world
GetInputManager:AddBinding("control,shift,r,game_and_editor", "OnRestartEngine", C_ENTITY_NONE);


GetInputManager:AddBinding("tab,always", "OnTurboSpeed", C_ENTITY_NONE);

//let's map a bunch of buttons for now for the 'speed up function'
GetInputManager:AddBinding("joy_any_button_4", "OnTurboSpeed", C_ENTITY_NONE); //let a button do speed up as well
GetInputManager:AddBinding("joy_any_button_5", "OnTurboSpeed", C_ENTITY_NONE); //let a button do speed up as well

GetInputManager:AddBinding("joy_any_button_7", "OnKeyEscape", C_ENTITY_NONE); //let a button do speed up as well
GetInputManager:AddBinding("joy_any_button_11", "OnKeyEscape", C_ENTITY_NONE); //let a button do speed up as well

//Note:  More editor keys are setup in editor.lua
	
GetCamera:Reset(); //undo any weird lerp or scale settings we had


function OnTurboSpeed(bKeyDown)

	if (bKeyDown) then
//		LogMsg("Turbo on!");
		GetApp:SetSimulationSpeedMod(2.4);
	else
//		LogMsg("Turbo off");
		GetApp:SetSimulationSpeedMod(1.0);
	
	end

	return false; //don't process any other callbacks for this key press

end

function OnToggleEditMode(bKeyDown)

	if (bKeyDown) then
		GetGameLogic:ToggleEditMode();
	end

		return false; //don't process any other callbacks for this key press
end

function OnRestartEngine(bKeyDown)

if (bKeyDown) then
	GetGameLogic:SetRestartEngineFlag(true);
	end

	return false; //don't process any other callbacks for this key press
end


