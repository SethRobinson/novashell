//some vars we want to keep track of per script

m_ownerID = C_ENTITY_NONE;
 
function OnInit() //run upon initialization
	
	this:SetVisualProfile("~/bullet.xml", "bullet");
	this:LoadCollisionInfo("~/bullet.col");
    
end

function OnPostInit() //run during the entity's first logic update
	//respond to mouse clicks to shoot
	this:SetAnimFrame(0);
	this:SetAnimPause(true);
	this:SetCategory(C_CATEGORY_DAMAGE, true);
	this:SetCategory(C_CATEGORY_ENTITY, false)
	this:SetLayerByName("Overlay 1"); //so the bullet can be seen over the enemies
	this:SetCollisionCategories(C_CATEGORIES_NONE); //clear it out, collide with nothing
	this:SetCollisionCategory(C_CATEGORY_ENEMY, true); 
	this:SetCollisionCategory(C_CATEGORY_TILEPIC, true); //the walls
	
	this:SetCollisionListenCategory(C_CATEGORY_ENEMY, true); //know when we bump into cans and things
	this:SetCollisionListenCategory(C_CATEGORY_TILEPIC, true); //the walls
	
	this:SetAcceleration(25); //get up to speed fast
end

function OnKill() //run when removed
end

function SetDirectionAndOwnerID(vDir, ownerID)

	m_ownerID = ownerID;
	
	//let's use a brain to control movement.  That way, later we can just
	//edit his visual profile and the bullet can have different animations for each
	//direction automatically if we want to do that.
	
	this:GetBrainManager():Add("StandardBase", "");
	this:GetBrainManager():SetStateByName("Walk");
	this:SetVectorFacingTarget(vDir);
	this:SetVectorFacing(vDir);
	
	//this:SetMaxMovementSpeed(80); //the maximum speed we can go
	this:SetDesiredSpeed(25); //how fast we actually want to go

	Schedule(5000, this:GetID(), "this:SetDeleteFlag(true)"); //kill ourselves in a bit
	
end

function Explode()
	
		//LogMsg("Running explode for ", this:GetID(), " at ", GetApp:GetGameTick());
	 	this:SetAnimPause(false);
		this:SetCollisionCategories(C_CATEGORIES_NONE); //don't get any more callbacks
		this:SetCollisionListenCategories(C_CATEGORIES_NONE);
		this:GetBrainManager():SetStateByName("Idle"); //stop moving
		
		//If we wanted to stop the bullet movement completely we could do this.  But we don't,
		//because we want it to keep pushing the can for a physics effect
		this:Stop();

		//Note, another way we could have stopped movement, except without affecting angular movement:
		//this:AddForce( (this:GetVectorFacing()*-1) * this:GetLinearVelocity():Length());

		
end

function OnCollisionStatic(normal, depth, materialID, ent) //return true/false to allow/disallow the collision

//	LogMsg("Hit a wall! Normal of collision is " .. tostring(normal) .. " type is ", materialID);
	
	if (normal.y > 0.8) then
		//this means we hit the ceiling.  If I don't test the angle, it's possible to hit the
		//sides of the shooting range causing the bullet to explode right away which looks
		//kind of strange
		
		Explode();
	end;
	
	return; //collision is valid
end


//vPosition, vVelocity and vNormal are Vector2's, depth and materialID are numbers, entity is a Entity, and state let's you know
//if this is a first contact or a persisting one. (Don't worry about state for now, will add more later..)
//if entity is nil, it is a collision with a TilePic

function OnCollision(vPosition, vVelocity, vNormal, depth, materialID, entity, state)

	if (entity == nil) then
		//it's a tilepic wall, explode
		Explode();
		return;
	end

	local entType = entity:Data():Get("type");
	 
	if (entType != "Gun" and entType != "Bullet") then
	 	//we'll, we're not touching the gun or another bullet, must be an enemy
		//LogMsg("We hit a " .. entType .. "!");
	 	//tell the thing we hit that they got damage so they can die or change color or whatever
		//LogMsg("Running on hit for", this:GetID());
		entity:RunFunction("OnHit", 1);
		
		Explode();
		return; 
	end
	
end

