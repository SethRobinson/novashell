
function OnInit() //run upon initialization

	this:SetVisualProfile("~/barrel.xml", "barrel");

	if (this:Data():Exists("broken")) then
		//we're already destroyed
		LogMsg("Setting anim broken..");
		this:SetAnimByName("broken");
		
	else
		this:SetAnimByName("break");
		this:SetAnimPause(true); //stop at frame 1
		this:LoadCollisionInfo("~/barrel.col");
		this:SetDensity(2); //make us harder to push around
		this:SetDampening(0.08); //so we don't slide everywhere
	end

	
end

function OnPostInit() //run during the entity's first logic update

	
	if (this:Data():Exists("broken") == false) then

	//let's force ourselves to be on the entity layer

		layerID = this:GetMap():GetLayerManager():GetLayerIDByName("Entity");
		if (layerID != C_LAYER_NONE) then
			this:SetLayerID(layerID);
		else
			LogError("Why the heck don't you have a layer named entity?");
		end
	end
	
end



function OnKill() //run when removed

end

//entParent is whoever sent the damage, entObject is the actual damaging object/projectile if not nil
function OnDamage(normal, depth, entParent, strength, extra, entObject)
	LogMsg("hitting barrel..");
	this:SetAnimPause(false); //play the rest of the anim
	this:PlaySoundPositioned("audio/effects/crate_break.ogg");

	//either way works
	this:LoadCollisionInfo(""); //clear collision data
	this:SetLayerID(this:GetLayerID()-1); //change the layer to one less, so entities will walk ON it instead of depth sort it
	
	//make sure we stay broken after loading a saved game:
	this:Data():Set("broken", "");
	
end


function OnAction(ent)

	GetTextManager:Add("Nice barrel.", ent);
  
end