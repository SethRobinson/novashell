//this script is run from start.lua in base when the engine is starting.

//assign some keys..

GetInputManager:AddBinding("r,editor_only", "OnEditorResetCameraScale", C_ENTITY_NONE);
GetInputManager:AddBinding("shift,r,editor_only", "OnEditorResetCamera", C_ENTITY_NONE);
GetInputManager:AddBinding("mouse_wheel_up,editor_only", "OnEditorMouseWheelUp", C_ENTITY_NONE);
GetInputManager:AddBinding("mouse_wheel_down,editor_only", "OnEditorMouseWheelDown", C_ENTITY_NONE);

GetInputManager:AddBinding("tab,editor_only,always", "OnEditorTab", C_ENTITY_NONE);
GetInputManager:AddBinding("escape,editor_only", "OnEditorKeyEscape", C_ENTITY_NONE);


GetInputManager:AddBinding("subtract,editor_only,always", "OnEditorKeyZoomDown", C_ENTITY_NONE);
GetInputManager:AddBinding("equals,editor_only,always", "OnEditorKeyZoomUp", C_ENTITY_NONE);

//extra bindings for zoom for laptops or whatever.  The "always" means trigger even if shift/control/alt/etc
//are also down.
GetInputManager:AddBinding("numpad_subtract,editor_only,always", "OnEditorKeyZoomDown", C_ENTITY_NONE);
GetInputManager:AddBinding("numpad_add,editor_only,always", "OnEditorKeyZoomUp", C_ENTITY_NONE);

//GetInputManager:AddBinding("g,game_and_editor,always", "OnGarbageCollect", C_ENTITY_NONE);

//testing thing I was using..
function OnGarbageCollect(bKeyDown)

if (bKeyDown) then
		LogMsg("Doing garbage collection...");
		collectgarbage();
	end

	return false;
end
//if escape is hit while the editor is active and we're editing in a saved game, just toggle it off

function OnEditorKeyEscape(bKeyDown)

  if (GetGameLogic:UserProfileActive() == false) then
  	//we're NOT editing a saved game..  hmm. what to do.. nothing I guess
	  return false; //don't process any other callbacks for this keypress
  end
	
  if (bKeyDown) then
 		GetGameLogic:ToggleEditMode();
	end
	
	  return false; //don't process any other callbacks for this keypress
end


function OnEditorTab(bKeyDown)
	if (bKeyDown) then
		
		local ent = GetSpecialEntityByName("editor");
		if (ent) then
		   ent:Send("toggle_hide");		
		end
	end
	
	return true;
end


function EditorCameraZoom(bZoomCloser)

	vScale = GetCamera:GetScale();
	scaleSpeed = 0.85;
	
	if (bZoomCloser) then
		vScale = vScale / scaleSpeed;
	else
		vScale = vScale * scaleSpeed;	
	end
		
		GetCamera:SetScale(vScale);
		GetCamera:InstantUpdate(); //don't let it lerp slowly
end

function OnEditorMouseWheelUp(bKeyDown)
	
	if (bKeyDown) then
		EditorCameraZoom(true); //zoom in
	end
		
	return true; //process any other remaining callbacks
end

function OnEditorKeyZoomUp(bKeyDown)
	
	if (bKeyDown and not g_consoleActive) then
		EditorCameraZoom(true); //zoom in
	end
		
	return true; //process any other remaining callbacks
end

function OnEditorMouseWheelDown(bKeyDown)
		if (bKeyDown) then
		EditorCameraZoom(false); //zoom out
	end

	return true; //process any other remaining callbacks
end

function OnEditorKeyZoomDown(bKeyDown)
	
	if (bKeyDown and not g_consoleActive) then
		EditorCameraZoom(false); //zoom out
	end
		
	return true; //process any other remaining callbacks
end

function OnEditorResetCameraScale(bKeyDown)
		if (g_consoleActive) then return true; end; //don't do it when typing in the console
		//let's reset the zoom to 1/1 with this key

		GetCamera:SetScale(Vector2(1,1));
		GetCamera:InstantUpdate(); //don't let the camera leisurely lerp to the target
	
	return true; //process any other remaining callbacks
end

function OnEditorResetCamera(bKeyDown)

		if (g_consoleActive) then return true; end; //don't do it when typing in the console

		//editor is active!
		GetCamera:Reset(); //resets scale, position, lerp settings
		GetCamera:InstantUpdate(); //don't let the camera leisurely lerp to the target

	return true; //process any other remaining callbacks
end


function OnOpenEditor()
	LogMsg("Editor opened up");
	SetRetailMode(false); //if we're opening the editor, we need to disable retail mode
	//if it wasn't already, so the F1 toggle will become active so we can close it
	
	g_bEditorRestartMusic = g_SmartSound:FadeOutCurrentMusic();
	g_bEditorRestartAmbience = g_SmartAmbience:FadeOutCurrentMusic();	
	g_editorActive = true;
	GetSoundManager:Play("audio/system/editor.ogg");
	g_bLastCursorOn = GetApp:GetCursorVisible();
	GetApp:SetCursorVisible(true);
end


function OnCloseEditor()
	GetApp:SetCursorVisible(g_bLastCursorOn);
	g_editorActive = false;
	
	if (g_bEditorRestartAmbience) then
		g_SmartAmbience:FadeInLastPlayedMusic();	
	end
	if (g_bEditorRestartMusic) then
		g_SmartSound:FadeInLastPlayedMusic();	
	end
	LogMsg("Editor closed");
end