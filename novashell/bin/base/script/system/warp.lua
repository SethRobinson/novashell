
function OnInit() //before the entity is placed in the world, init visuals here, so it knows the size dimensions
	this:SetTrigger(C_TRIGGER_TYPE_REGION_IMAGE, 0, C_TRIGGER_BEHAVIOR_NORMAL, 0);

	//init persistent vers
	this:Data():SetIfNull("Target", "");
	this:Data():SetIfNull("WarpSound", "");
	this:SetNavNodeType(C_NODE_TYPE_WARP);
	this:SetHasPathNode(true);
end

function OnMapInsert() //run after OnInit, but before PostInit

	GetTagManager:RegisterAsWarp(this, this:Data():Get("Target")); 
end

function IsAWarp()
 return true;
end


function OnTriggerEnter(ent)
 
  if (ent:GetGoalManager():IsGoalActiveByName("MapAndPosMove") == true) then
  	//this entity is already in the middle of some move AI, don't screw with it by warping
  	return;
 end
  
  entTarget = GetEntityByName(this:Data():Get("Target"));
  
  if (entTarget == nil) then
   	LogError("Can't find " .. this:Data():Get("Target") .. ".  Maybe it doesn't exist yet?");
   return;
  end
  
  offset = Vector2(0,0);
  
  if (entTarget:RunFunction("IsAWarp")) then
	  //move him off the warp pad in the way he was facing?
	  size = entTarget:GetSizeX();
	  if (entTarget:GetSizeY() > size) then size = entTarget:GetSizeY(); end;
	  
	  size = size + 40;
	  offset = ent:GetVectorFacing() * size;
	  offset.x = offset.x * entTarget:GetScale().x;
	  offset.y = offset.y * entTarget:GetScale().y;
	 end

	entTarget:RunFunction("OnGoalPreWarp", ent);

    ent:SetPosAndMap(entTarget:GetPos()+  offset , entTarget:GetMap():GetName());
	entTarget:RunFunction("OnGoalPostWarp", ent);
  
end

function OnTriggerExit(ent)
end

function OnGoalPreWarp(ent)
	
	local soundToPlay = this:Data():Get("WarpSound");
	
	if (soundToPlay == "") then
		//use the default
		soundToPlay = "audio/effects/door_open.ogg";
	end

	ent:PlaySoundPositioned(soundToPlay);

end

function OnGoalPostWarp(ent)
	if (this:Data():Get("WarpSound") == "") then
		Schedule(200, ent:GetID(), 'this:PlaySoundPositioned("audio/effects/door_close.ogg");');  
	end
end