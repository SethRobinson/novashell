
function OnInit()
	
	//init persistent vers
	this:Data():SetIfNull("Target", "");
	this:Data():SetIfNull("Source", ""); //optional, if defined, we'll move ourselves to this exact spot BEFORE the warp, helps when coming
	//back so we don't instantly warp somewhere because we're sitting on the warp spot
	this:Data():SetIfNull("OnActionText", "It's a door.");
this:SetDensity(0); //makes it immovable, speeds up processing too
		
end

function OnWarp(Ent)

  playerTag = GetTagManager:GetFromString(this:Data():Get("Target"));
	
	if (playerTag == nil) then
		 LogError("Can't locate the target '" .. this:Data():Get("Target") .. "'  Note that it's case sensitive");
      return;
		 end
	 
	 if (this:Data():Get("Source") != "") then
	  //the source var has data, try to move the original person here
	  Ent:SetPosAndMapByTagName(this:Data():Get("Source"));
	 end
	 
  
	  GetSoundManager:Play("audio/warp.ogg");
	 
	 
	  GetGameLogic:SetPlayer(nil); //kill the current player focus, if where we're going has a player, it will kick in automatically now
     
     //additionally, let's carry over the camera settings instead of using the maps default, as no player is selected it won't happen
     //automatically now
     
      camSettings = GetCamera:GetCameraSettings();
     
      mapName = playerTag:GetMapName();
      GetMapManager:SetActiveMapByName(mapName);
      //LogMsg("Moving to " .. tostring(playerTag:GetPos()));
      
      GetCamera:SetCameraSettings(camSettings);
      
      GetCamera:SetPosCentered(playerTag:GetPos());
      GetCamera:InstantUpdate();

end

function OnAction(enemy)

	GetTextManager:Add(this:Data():Get("OnActionText"), enemy);
   
end




