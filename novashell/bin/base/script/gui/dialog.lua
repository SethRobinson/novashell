//Helper functions for dialog.  Override this file if you want to get fancy
g_dialogCanBeCanceled = true; //this is the default.  By setting it to false right before you create a dialog,
//you can make sure it is unskippable.  (after the dialog is closed, it will be set back to true again automatically)


//map the keyboard geys that can be used for dialog

GetInputManager:AddBinding("up", "OnDialogMoveUp", C_ENTITY_NONE);
GetInputManager:AddBinding("down", "OnDialogMoveDown", C_ENTITY_NONE);
GetInputManager:AddBinding("return", "OnDialogChoose", C_ENTITY_NONE);


//map the joystick and keys that can also be used for dialog..

GetInputManager:AddBinding("joy_any_up", "OnDialogMoveUp", C_ENTITY_NONE);
GetInputManager:AddBinding("joy_any_down", "OnDialogMoveDown", C_ENTITY_NONE);
GetInputManager:AddBinding("joy_any_button_0", "OnDialogChoose", C_ENTITY_NONE);
GetInputManager:AddBinding("joy_any_button_1", "OnDialogKeyEscape", C_ENTITY_NONE); //cancel button


 
function SendToActiveDialog(msg)
					dialog = GetSpecialEntityByName("ChoiceDialog");
			if (dialog != nil) then
				dialog:Send(msg);
				end
end
	
function OnDialogMoveUp(keyDown)
	if (keyDown) then
				SendToActiveDialog("mod_selection|-1");
			end
	
	return true;
end

function OnDialogMoveDown(keyDown)
	if (keyDown) then
				SendToActiveDialog("mod_selection|1");
			end
	
	return true;
end

function OnDialogChoose(keyDown)
	if (keyDown) then
			SendToActiveDialog("select_active");
		
	end
	return true;

end

function OnDialogChangeSelection(dialog) //passed as a SpecialEntity

	 GetSoundManager:Play("audio/gui/change_selection1.ogg");

end


function OnDialogSelection(dialog) //passed as a SpecialEntity

	 GetSoundManager:Play("audio/gui/selection1.ogg");

end

function PlayDialogOpenSound()
		GetSoundManager:Play("audio/gui/dialog_open.ogg");
end

function OnDialogOpen(dialog) //passed as a SpecialEntity

	//let's override the escape key
	GetInputManager:AddBinding("escape", "OnDialogKeyEscape", C_ENTITY_NONE);
	
	if (GetPlayer) then
 		//well, let's give the player a chance to respond to the dialog close, for instance, to clear any pending keys so they
 		//don't re-start the dialog accidently right away or something
 	 		GetPlayer:RunFunctionIfExists("OnDialogOpenNotify", dialog);
	end
end

function OnDialogClose(dialog) //passed as a SpecialEntity
	 
	 //remove the escape key binding we had added
 	GetInputManager:RemoveBinding("escape", "OnDialogKeyEscape", C_ENTITY_NONE);
 	g_dialogCanBeCanceled = true; //set the default for next time
 	if (GetPlayer) then
 		//well, let's give the player a chance to respond to the dialog close, for instance, to clear any pending keys so they
 		//don't re-start the dialog accidently right away or something
 		GetPlayer:RunFunctionIfExists("OnDialogCloseNotify", dialog);
 	end
end

//this is called when they hit Escape while a dialog is going
function OnDialogKeyEscape(bKeyDown)

	if (bKeyDown) then
		//User hit escape while a dialog was on the screen.
		if (g_dialogCanBeCanceled) then
			dialog = GetSpecialEntityByName("ChoiceDialog");
			if (dialog != nil) then
				//well, we found a dialog.. assume this is it?
				
				//we might as well let the dialog handler know that the user hit escape as well, maybe they will want to do something with it.
				local callbackFuncName = dialog:Send("get_callback");
				local entID = tonumber(dialog:Send("get_entity_id"));
				local selected = dialog:Send("get_selection");
				
				if (entID == C_ENTITY_NONE) then
					//it's a global function
					loadstring(callbackFuncName .. '("' .. selected .. '","_escape_", dialog)')();
				else
					//it's inside an entity's namespace where we need to call the function
					GetEntityByID(entID):RunFunction(callbackFuncName, selected, "_escape_", dialog);
				end
				//LogMsg("Canceling dialog");
				dialog:SetDeleteFlag(true);
			
			
			end
		else
			//we've been told this dialog is unskippable.  Play a sound effect instead?
		end
			
	end
	
	return false; //important, this means everything that was bound EARLIER than us for the escape key
	//will *NOT* be run.  This is what we want in this case, to override quitting the game or whatever.  All
	//we want to do is cancel the dialog box.
end