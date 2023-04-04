
function OnInit() //run upon initialization
	
	this:SetVisualProfile("~/can.xml", "can");
	this:LoadCollisionInfo("~/can.col");
	this:SetEnableRotationPhysics(true);
	this:SetScale(Vector2(0.4, 0.4)); //be smallish
end

function OnPostInit() //run during the entity's first logic update

	this:SetCategory(C_CATEGORY_DAMAGE, true);
	this:SetCategory(C_CATEGORY_ENTITY, false)
	this:SetLayerByName("Overlay 1"); //so the bullet can be seen over the enemies
	this:SetCollisionCategories(C_CATEGORIES_NONE); //clear it out, collide with nothing
	this:SetCollisionCategory(C_CATEGORY_PLAYER, true); //add just the player category
	this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true); //know when we bump into cans and things
	
	
	//let's use a brain to control movement.  That way, later we can just
	//edit his visual profile and the bullet can have different animations for each
	//direction automatically if we want to do that.
	
	this:GetBrainManager():Add("StandardBase", "");
	this:GetBrainManager():SetStateByName("Walk");
	vDir = Vector2(0, 1); //down
	this:SetVectorFacingTarget(vDir);
	this:SetVectorFacing(vDir);
	
	this:SetMaxMovementSpeed(80); //the maximum speed we can go
	this:SetDesiredSpeed(80); //how fast we actually want to go

	Schedule(5000, this:GetID(), "this:SetDeleteFlag(true)"); //kill ourselves in a bit

	this:Data():Set("type", "Bullet"); //changing this would let the player shoot these down
	
	this:SetBaseColor(Color(50, 255, 50, 255)); //look green so we stand out
	//play a sfx
	this:PlaySound("audio/bomb_drop.ogg");
	
	this:SetRotation(random(0,6.14)); //choose a random angle to start the can out as
	
end


function OnHit(damage) //run when a bullet notifies us that we got hit
	 	
	 		//we're dead!
		 	this:GetBrainManager():Add("FadeOutAndDelete", "fade_speed_ms=50");
			this:SetCollisionCategories(C_CATEGORIES_NONE);
			this:SetCollisionListenCategories(C_CATEGORIES_NONE);
			m_hitpoints = 0;
	
end

//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)
//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

  //this is old outdated stuff, could be done smarter by just using categories or 
  //entity:GetType()
  
	local entType = entity:Data():Get("type");
	 
	if (entType == "Gun") then
	 	//we'll, we're not touching the gun or another bullet, must be an enemy
		//LogMsg("We hit a " .. entType .. "!");
	 	//tell the thing we hit that they got damage so they can die or change color or whatever
		entity:RunFunction("OnHit", 1);
		OnHit(1); //make sure we die too

		return ; //well, if we said true here, we'd push back the player which we don't want
	end
	
end


function OnKill() //run when removed
end

