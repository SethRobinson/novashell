//this can operate two ways, one, you assign it to a pic and setup a collision line as "warp" type.
//Notice that it's limited to rect collision checking.
//The second and more accurate method, is it will warp on touch if the collision line is set to "normal" property.

function OnInit()
	
	//init persistent vers
	this:Data():SetIfNull("Target", "");
	this:Data():SetIfNull("OnActionText", "It's a door.");
	//this:SetNavNodeType(C_NODE_TYPE_WARP);
	//this:SetHasPathNode(true);
	this:SetDensity(0); //makes it immovable, speeds up processing too
	
	//this:SetListenCollision(C_LISTEN_COLLISION_PLAYER_ONLY);
end

function OnPostInit()
end

function OnWarp(Ent, vOffset) //the offset parm is optional

  Ent:PlaySoundPositioned("audio/effects/door_open.ogg");
  
  local tag = GetTagManager:GetFromString(this:Data():Get("Target"));
  if (tag == nil) then
  	LogError("Tag " .. this:Data():Get("Target") .. " doesn't seem to exist yet.");
  	return;
  end
  
  local vDestPos = tag:GetPos();
  
  if (vOffset) then
  	//additional placement tweak requested
  	vDestPos = vDestPos +vOffset;
  end

  Ent:PlaySoundPositioned("audio/effects/door_close.ogg");
  Ent:SetPosAndMap(vDestPos, tag:GetMapName());
end

function OnAction(enemy)

	GetTextManager:Add(this:Data():Get("OnActionText"), enemy);
   
end


function OnCollision(normal, depth, materialID, enemy) //return true/false to allow/disallow the collision
	
	LogMsg("On warp got its own collision, but it isn't listening!");
	OnWarp(enemy);	
	return false;
end





