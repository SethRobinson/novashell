//some vars we want to keep track of per script

m_ownerID = C_ENTITY_NONE;
m_damage = 1;

function OnInit() //run upon initialization
end

function OnPostInit() //run during the entity's first logic update
	//this:SetAnimFrame(0);
	//this:SetAnimPause(true);
	
	if (this:GetMap():GetName() == "Kill Palette") then
		//don't do any more initialization here, we're just a template version
		return;
	end
	
	this:SetDensity(0.4); 
	this:SetType(C_TYPE_DAMAGE);
	m_bExploding = false;
	this:SetCategory(C_CATEGORY_ENTITY, false); //don't show up on these scans
	this:SetCategory(C_CATEGORY_BULLET, true); //we're a bullet

end

function OnKill() //run when removed
end

function SetSpeed( speed)
	this:SetMaxMovementSpeed(speed); //the maximum speed we can go
	this:SetDesiredSpeed(speed); //how fast we actually want to go
	this:SetAcceleration(speed); //how fast our bullet reaches top speed

end

function SetDirectionAndOwnerID(vDir, ownerID, damage)

	local speed = 20;
	this:SetName("");

	m_ownerID = ownerID;
	m_damage = damage;
	//GetWatchManager:Add(this, C_TIME_FOREVER);
  this:SetCollisionCategories(C_CATEGORIES_NONE); //clear this out first
  this:SetCollisionCategory(C_CATEGORY_TILEPIC, true); //stop on walls
	this:SetCollisionListenCategory(C_CATEGORY_TILEPIC, true); //know when we bump into walls

	if (ownerID == g_playerID) then
		//it's the player shooting this.  Let's collide with enemies only
		this:SetCollisionCategory(C_CATEGORY_ENEMY, true); //collide with enemies
	  this:SetCollisionListenCategory(C_CATEGORY_ENEMY, true); //get notification when we collide with enemies
	  speed = 50; //let the player shoot faster
	else
		//enemy shooting at the player 
		this:SetCollisionCategory(C_CATEGORY_PLAYER, true); //collide with the player
	  this:SetCollisionListenCategory(C_CATEGORY_PLAYER, true); //get notification when we collide with the player

	end
	
	
	this:SetVisibilityNotifications(true); //OnGetVisible and OnLoseVisible will get called now
	
	//let's use a brain to control movement.  That way, later we can just
	//edit his visual profile and the bullet can have different animations for each
	//direction automatically if we want to do that.
	this:SetTurnSpeed(speed); //instant
	this:SetMaxMovementSpeed(speed); //the maximum speed we can go
	this:SetDesiredSpeed(speed); //how fast we actually want to go
	this:SetAcceleration(speed); //how fast our bullet reaches top speed
	
	this:SetPersistent(false); //don't save

	this:GetBrainManager():Add("StandardBase", "");
	this:GetBrainManager():SetStateByName("Walk");
	this:SetVectorFacingTarget(vDir);
	this:SetVectorFacing(vDir);
	//this:SetEnableRotationPhysics(true);
	this:SetRotation(math.atan2(vDir.x, -vDir.y));
	
	Schedule(3000, this:GetID(), "this:SetDeleteFlag(true)"); //kill ourselves in a bit
	
end

function OnLoseVisible() //we left the screen

  //we're off screen
  this:SetDeleteFlag(true);
end

function OnGetVisible() 
end

function Explode()
	
		this:SetCollisionListenCategories(C_CATEGORIES_NONE);
		this:SetCollisionCategories(C_CATEGORIES_NONE);
		
		this:GetBrainManager():SetStateByName("Idle"); //stop moving
		
		//If we wanted to stop the bullet movement completely we could do this.  But we don't,
		//because we want it to keep pushing the can for a physics effect
		//this:Stop();

		//Note, another way we could have stopped movement, except without affecting angular movement:
		//this:AddForce( (this:GetVectorFacing()*-1) * this:GetLinearVelocity():Length());
		m_bExploding = true;
		//Schedule(400, this:GetID(), "this:SetDeleteFlag(true)"); //kill ourselves very soon
		this:SetDeleteFlag(true);
		
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
	
	 	//tell the thing we hit that they got damage so they can die or change color or whatever
		entity:RunFunctionIfExists("OnHit", m_damage);
		Explode();
end

