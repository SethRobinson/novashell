function OnInit()
	
	this:Data():SetIfNull("Text", "None");


	if (GameIsActive()) then
		//not in edit mode, activate this
		this:SetTrigger(C_TRIGGER_TYPE_REGION_IMAGE, 0, C_TRIGGER_BEHAVIOR_NORMAL, 0);
	end

end

function OnTriggerEnter()
 
 //GetSoundManager:Play("audio/long_view.ogg");
 
 GetCamera:SetMoveLerp(1);
 GetCamera:SetScaleLerp(0.04);
 GetCamera:SetScaleTarget(Vector2(5,5)); 

 Schedule(250, C_ENTITY_NONE, 'GetTextManager:Add("' .. this:Data():Get("Text") .. '", GetPlayer );');
 
 //schedule a global command since we'll be removed by then
 Schedule(1500, C_ENTITY_NONE, "GetCamera:SetScaleTarget(Vector2(1,1));GetCamera:SetScaleLerp(0.04);");

 //disable us
 this:SetDeleteFlag(true);
end


function OnTriggerExit()
 GetCamera:SetScaleTarget(Vector2(1,1)); 

end